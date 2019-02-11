/**********************************************************************************************************************
*
*	Terrain.cpp
*	Class for loading the .asc terrain pointclouds, parsing them and bundling them in to a format that is useful to
*	OpenGL, and rendering them.
*
*	Changelog
*	21.08.2018, Peter - now loads all terrain chunks within a 100km square of the camera
*	23.08.2018, Peter - Unloading terrain chunks is now a thing!
*	24.08.2018, Peter - Added in some background stuff to make asynchoronous loading easier when I get round to it.
*	27.08.2018, Peter - Added an unload() method to clean out memory when a chunk is discarded. ENORMOUS improvement
*	in program framerates and stability :tada:
*	08.10.2018, Peter - Fixed Terrain loading/unloading detection for chunks
*
***********************************************************************************************************************/


#include "stdHeaders.h"
#include "Variables.h"
#include "Terrain.h"
#include "boost\algorithm\string.hpp"
#include <deque>

/*************************************************************************
*
*	Terrain Chunk Class
*	Stores the high-resolution chunk for each block of terrain
*	Responsible for supersampling for LOD models
*
**************************************************************************/


TerrainChunk::TerrainChunk() {
}
TerrainChunk::~TerrainChunk() {
}
std::string TerrainChunk::setName(std::string newName) {
	std::string oldName = name;
	name = newName;
	return oldName;
}
glm::vec3 TerrainChunk::setPos(glm::vec3 newPos) {
	glm::vec3 oldPos = pos;
	pos = newPos;
	return oldPos;
}

glm::vec2 TerrainChunk::getTopLeftCorner() {
	glm::vec2 topLeftCorner(pos.x, pos.y+10000);
	return topLeftCorner;
}
glm::vec2 TerrainChunk::getBottomRightCorner() {
	glm::vec2 bottomRightCorner(pos.x+10000, pos.y);
	return bottomRightCorner;
}

int TerrainChunk::getChunkState() {
	return chunkState;
}
int TerrainChunk::setChunkState(int newState) {
	int oldState = chunkState;
	if (newState <= 3 && newState >= 0) {
		chunkState = newState;
	}
	else{
		std::cout << "ERROR: TRIED TO SET INVALID CHUNK STATE VALUE" << std::endl;
	}
	return oldState;
}

int TerrainChunk::reqLoadUnloadChunk(bool load, int numTerrainChunksCurrentlyLoading, std::string reason) {
	if (load) {
		if (chunkState == 0 && numTerrainChunksCurrentlyLoading < 2) { //if it is empty and less than two other chunks are currently loading...
			startAsyncLoadFile();
			return 1;
		}
	}
	else {
		if (chunkState == 2) { //if the chunk is loaded, we can unload it!
			std::cout << "Chunk Unload because '" << reason << "'" << std::endl;
			unload();
			return 0;
		}
	}
	return 0;
}

int TerrainChunk::startAsyncLoadFile() {
	chunkState = 1;
	//std::cout << "Thread to load terrain chunk '" << name << "' started!" << std::endl;
	std::string *tileName = &name;
	threadHandle = CreateThread(0, 0, asyncLoadFile, this, 0, &threadID);
	if (threadHandle == NULL) {
		std::cout << "ERROR: FAILED TO CREATE NEW THREAD TO LOAD TERRAIN CHUNK" << std::endl;
		chunkState = 0;
		return 0;
	}
	return 1;
}

DWORD WINAPI TerrainChunk::asyncLoadFile(__in LPVOID lpParameter) { //lpParameter is the terrain chunk name
	TerrainChunk *thisChunk = (TerrainChunk*)lpParameter;
	std::string chunkName = thisChunk->getName();
	TerrainChunk editingChunk;
	glm::vec2 foundChunk(-1, -1);
	bool thrownError = false;
	// Find which chunk that name applies to...
	for (unsigned int j = 0; j < g_terrain.size(); j++) {
		if (g_terrain[j].getName() == chunkName.substr(0, 2)) { //We found the right major gridsquare
			for (int k = 0; k < int(g_terrain[j].chunkList.size()); k++) {
				if (g_terrain[j].chunkList[k].getName() == chunkName) {
					editingChunk = g_terrain[j].chunkList[k];
					foundChunk = glm::vec2(j, k);
					break;
				}
			}
			break;
		}
	}
	if (foundChunk.x != -1 && foundChunk.y != -1) {
		if (editingChunk.name != "" && editingChunk.name.length() == 4) {
			std::cout << "Async Loading Terrain Chunk: " << editingChunk.name << "." << std::endl;
			int retVal = editingChunk.loadFile("Data\\Maps\\OS\\data\\asc\\" + editingChunk.getName() + ".asc");
			if(retVal == 1){
				if (editingChunk.genSurfaceVertices()) {
					if (editingChunk.genSurfaceIndices()) {
						editingChunk.setChunkState(2);
					}
				}
				else {
					std::cout << "ASYNC ERROR: Failed to gen chunk verts for '" << editingChunk.name << "'." << std::endl;
					thrownError = true;
				}
			}
			else if (retVal == -1) {
				std::cout << "WARNING: Chunk file '" << editingChunk.name << "' non extant!" << std::endl;
				editingChunk.setChunkState(3);
			}
			else{
				std::cout << "ASYNC ERROR: Failed to load chunk file '" << editingChunk.name << "'." << std::endl;
				thrownError = true;
			}
		}
		else {
			std::cout << "ASYNC ERROR: Chunk '" << editingChunk.name << "' failed string test!" << std::endl;
			thrownError = true;
		}
		if (thrownError) {
			std::cout << "ERROR: Load Error detected. Unloading." << std::endl;
			editingChunk.unload();
		}
		editingChunk.doneLoadingFlag = true;
		// Apply changes to the chunk
		g_terrain[foundChunk.x].chunkList[foundChunk.y] = editingChunk;
		if (thrownError) {
			std::cout << "Failed loading Terrain Chunk " << editingChunk.name << "!" << std::endl;
		}
		else {
			std::cout << "Successfully loaded Terrain Chunk " << editingChunk.name << "!" << std::endl;
		}
	}
	return 1;
}

int TerrainChunk::loadFile(std::string filePath) {
	std::ifstream heightmap; //open file to read data
	heightmap.open(filePath, std::ios::binary);
	if (heightmap.is_open()) {
		//slurp into RAM
		//std::cout << "Loading heightmap for " << name << "." << std::endl;
		std::stringstream sstr;
		sstr << heightmap.rdbuf();
		heightmap.close();
		//std::cout << "Done loading heightmap for " << name << "." << std::endl;
		//std::cout << "Parsing heightmap for " << name << "." << std::endl;
		//Now split the string into lines
		if (sstr.str().length() > 0) {
			std::deque<std::string> heightmapLines = util_string_split(sstr.str(), char('\n'));
			int i = 0;
			for (std::deque <std::string>::iterator it = heightmapLines.begin(); it != heightmapLines.end(); it++) {
				if (*it != "" && it->length() > 1) { //Check it's a plausable string to avoid throwing up a warning
					std::deque<std::string> stringVals = util_string_split(*it, ' ');
					if (stringVals.size() > 5) { //does it contain enough data to be a vertex row?
						std::vector<int> thisLine;
						int j = 0;
						for (std::deque <std::string>::iterator it2 = stringVals.begin(); it2 != stringVals.end(); it2++) {
							if (*it2 != "" && *it2 != " ") {
								thisLine.emplace_back(std::stoi(*it2));
							}
							j++;
						}
						heightMapPointCloud.emplace_back(thisLine);
					}
					else { // We must be in the header!
						// Get tile south-west corner
						if (i == 2) {
							float xpos = stof(stringVals[1]);
							pos.x = xpos;
						}
						else if (i == 3) {
							float ypos = stof(stringVals[1]);
							pos.y = ypos;
						}
					}
				}
				i++;
			}
			path = filePath;
			int ref = path.find_last_of('\\') + 1;
			name = path.substr(ref, path.find_last_of('.') - ref);
			//std::cout << "Done Parsing file " << name << " @ " << pos.x << "N" << pos.y << "E!" << std::endl;
		}
		else {
			std::cout << "ERROR: Empty StringStream for '" << name << "'" << std::endl;
		}
	}
	else {
		return -1; //i.e. nonextant file!
	}
	return 1;
}

int TerrainChunk::genSurfaceVertices() {
	std::cout << "Start gen verts for chunk " << name << "." << std::endl;
	if (heightMapPointCloud.size() > 0) {
		int heightmapPointsNum = heightMapPointCloud.size()*heightMapPointCloud[0].size();
		heightmapVertices.reserve(heightmapPointsNum + 2);
		for (int i = 0; i < heightMapPointCloud.size(); i++) {
			std::vector<int> thisLine = heightMapPointCloud[i];
			for (int j = 0; j < thisLine.size(); j++) {
				float colour[4] = { float(thisLine[j]) / 500, float(thisLine[j]) / 500, float(thisLine[j]) / 500, 1.0 };
				if (float(thisLine[j]) <= 0) {
					colour[0] = 0.0f;
					colour[1] = 0.5f;
					colour[2] = 1.0f;
				}
				Vertex newVertex = { { (float(j) * 50), 10000 - (float(i) * 50), float(thisLine[j]), 1.0 },{ colour[0], colour[1], colour[2], colour[3] } };
				heightmapVertices.emplace_back(newVertex);
			}
		}
		std::cout << "Done gen verts for " << name << "." << std::endl;
	}
	else {
		std::cout << " FAILED. EMPTY POINT CLOUD FOR " << name << "." << std::endl;
		return 0;
	}
	return 1;
}
int TerrainChunk::genSurfaceIndices() {
	//We want to zig-zag between adjacent rows to make a single, continuous triangle strip
	std::cout << "Generating terrain mesh... ";
	int n = 0; //where are we on the current row?
	if (heightMapPointCloud.size() > 0) {
		const int len = heightMapPointCloud[0].size(); //how long is each row?
		int row = 0; //which row are we on?
		while (true) {
			if (float(row) / 2 == int(row) / 2) { //for even-numbered rows
				heightmapSurfaceIndices.emplace_back((row*(len)+n));
				heightmapSurfaceIndices.emplace_back(((row + 1)*(len)+n));
				n++;
				if (n >= len - 1) { //if we've reached the end of the row, move onto the next!
					n = len - 1;
					heightmapSurfaceIndices.emplace_back((row*(len)+n));
					row++;
					if (row >= heightMapPointCloud.size() - 1) {
						break;
					}
					heightmapSurfaceIndices.emplace_back(((row)*(len)+n));
				}
			}
			else { //for odd-numbered rows, we count backwards!
				heightmapSurfaceIndices.emplace_back(((row)*(len)+n));
				heightmapSurfaceIndices.emplace_back(((row + 1)*(len)+n));
				n--;
				if (n <= 0) { //if we've reached the end of the row, move onto the next!
					n = 0;
					heightmapSurfaceIndices.emplace_back(((row)*(len)+n));
					row++;
					if (row >= heightMapPointCloud.size() - 1) {
						break;
					}
					heightmapSurfaceIndices.emplace_back(((row)*(len)+n));
				}
			}
		}
		heightmapSurfaceIndices.push_back(heightmapSurfaceIndices[heightmapSurfaceIndices.size() - 1]);//duplicate the last index on the list
		heightMapPointCloud.clear(); //clear out some memory!
		std::cout << " done!" << std::endl;
	}
	else {
		std::cout << " FAILED. COULD NOT GEN SURF INDICES DUE TO EMPTY POINT CLOUD" << std::endl;
		return 0;
	}
	return 1;
}
int TerrainChunk::genSupersampledSurface(int resolution) {
	return 1;
}
int TerrainChunk::genBuffers() {
	if (heightmapVertices.size() > 0 && heightmapSurfaceIndices.size() > 0) {
		terrainShader = Shader(TerrainVertexShader, TerrainFragmentShader);

		GLenum ErrorCheckValue;

		GLenum GLErrorCheckValue = glGetError();

		glGenVertexArrays(1, &VaoId);
		glBindVertexArray(VaoId);

		glGenBuffers(1, &BufferId);
		glBindBuffer(GL_ARRAY_BUFFER, BufferId);
		glBufferData(GL_ARRAY_BUFFER, heightmapVertices.size() * sizeof(Vertex), heightmapVertices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(heightmapVertices[0]), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(heightmapVertices[0]), (GLvoid*)sizeof(heightmapVertices[0].XYZW));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &IndexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, heightmapSurfaceIndices.size() * sizeof(GLuint), heightmapSurfaceIndices.data(), GL_STATIC_DRAW);

		ErrorCheckValue = glGetError();
		if (ErrorCheckValue != GL_NO_ERROR) {
			//fprintf(stderr,"ERROR: Could not create a VBO: %s \n",glfwerror(ErrorCheckValue));
			std::cout << "TERRAIN VBO CREATION ERROR" << std::endl;
			return 0;
		}
	}
	// We no longer care about empty squares, that usually just means the chunk doesn't exist and is blank anyways. May need a better fix in future.
	/*else {
		std::cout << "ERROR: Terrain chunk '" << name << "' empty vertex list!" << std::endl;
		return 0;
	}*/
	return 1;
}

int TerrainChunk::unload(){
	if (chunkState == 2) {
		std::cout << "Unloading Terrain Chunk '" << name << "'." << std::endl;
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &VaoId);
		glDeleteBuffers(1, &IndexBufferId);
		glDeleteBuffers(1, &BufferId);
		heightmapSurfaceIndices.clear();
		terrainShader.deleteShader();
		chunkState = 0;
	}
	else {
		//std::cout << "ERROR: ILLEGAL TERRAIN UNLOAD REQUEST FOR '" << name << "'." << std::endl;
		return 0;
	}
	return 1;
}

int TerrainChunk::draw(glm::mat4 projection, glm::mat4 view) {
	GLenum ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR) {
		std::cout << "GLERROR1: " << ErrorCheckValue << std::endl;
		//ErrorCheckValue = glGetError();
		//std::cout << "GLERROR1a: " << ErrorCheckValue << std::endl;
	}
	if (chunkState == 2 && heightmapSurfaceIndices.size() > 0) {
		terrainShader.use(); //Activate terrain shader

		// pass projection matrix to shader (note that in this case it could change every frame)
		terrainShader.setMat4("projection", projection);

		ErrorCheckValue = glGetError();
		if (ErrorCheckValue != GL_NO_ERROR) {
			std::cout << "GLERROR2: " << ErrorCheckValue << std::endl;
			ErrorCheckValue = glGetError();
			std::cout << "GLERROR2a: " << ErrorCheckValue << std::endl;
		}

		glEnable(GL_DEPTH_TEST);
		glDepthRange(0.01, 10000);

		// camera/view transformation
		glm::mat4 model(1.0);
		model = glm::translate(model, pos);
		terrainShader.setMat4("model", model);
		terrainShader.setMat4("view", view);

		glBindVertexArray(VaoId);

		glDrawElements(GL_TRIANGLE_STRIP, heightmapSurfaceIndices.size(), GL_UNSIGNED_INT, NULL);

		ErrorCheckValue = glGetError();
		if (ErrorCheckValue != GL_NO_ERROR) {
			std::cout << "GLERROR3: " << ErrorCheckValue << std::endl;
			ErrorCheckValue = glGetError();
			std::cout << "GLERROR3a: " << ErrorCheckValue << std::endl;
		}
	}
	else {
		ErrorCheckValue = glGetError();
		if (ErrorCheckValue != GL_NO_ERROR) {
			std::cout << "GLERROR4: " << ErrorCheckValue << std::endl;
			//ErrorCheckValue = glGetError();
			//std::cout << "GLERROR4a: " << ErrorCheckValue << std::endl;
		}
		return 0;
	}
	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR) {
		std::cout << "GLERROR5: " << ErrorCheckValue << std::endl;
		//ErrorCheckValue = glGetError();
		//std::cout << "GLERROR5a: " << ErrorCheckValue << std::endl;
	}
	return 1;
}
std::string TerrainChunk::getName() {
	return name;
}
double TerrainChunk::getLowerLeftLat() {
	return lowerLeftCornerLat;
}
double TerrainChunk::getLowerLeftLong() {
	return lowerLeftCornerLong;
}
double TerrainChunk::getLateralSpacing() {
	return lateralPointSpacing;
}
double TerrainChunk::getLongitudinalSpacing() {
	return longitudinalPointSpacing;
}

/*************************************************************************
*	Terrain Superchunk class
*	Stores the largest terrain sections - one for each 100km gridsquare
**************************************************************************/
TerrainSuperchunk::TerrainSuperchunk() {
}

TerrainSuperchunk::~TerrainSuperchunk() {
}

std::string TerrainSuperchunk::getName() {
	return name;
}

int TerrainSuperchunk::initSuperchunk(std::string gridSquare, glm::vec2 position) {
	name = gridSquare;
	pos = position;
	dimensions = glm::vec2(100000, 100000);
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			std::string gridName = gridSquare+std::to_string(i) + std::to_string(j);
			TerrainChunk newChunk;
			newChunk.setName(gridName);
			newChunk.setPos(glm::vec3(pos.x+i*1000, pos.y+j*1000, 0));
			chunkList.emplace_back(newChunk);
		}
	}
	return 1;
}

int TerrainSuperchunk::manageTerrainChunks(glm::vec2 cameraPos, float viewRadius) {
	glm::vec2 topLeftCorner(pos.x, pos.y+dimensions.y);
	glm::vec2 bottomRightCorner(pos.x+dimensions.x, pos.y);
	if (util_isRectWithinCircle(cameraPos, viewRadius, topLeftCorner, bottomRightCorner)) { //should we consider loading some stuff?
		for (unsigned int i = 0; i < chunkList.size(); i++) {
			numTerrainChunksLoading += chunkList[i].reqLoadUnloadChunk(util_isRectWithinCircle(cameraPos, viewRadius,chunkList[i].getTopLeftCorner(), chunkList[i].getBottomRightCorner()), numTerrainChunksLoading, "chunk range");
		}
	}
	else {
		//everything should be unloaded!
		for (unsigned int i = 0; i < chunkList.size(); i++) {
			chunkList[i].reqLoadUnloadChunk(false, numTerrainChunksLoading, "superchunk out of range");
		}
	}
	return 1;
}

int TerrainSuperchunk::draw(glm::mat4 projection, glm::mat4 view) {
	glEnable(GL_DEPTH_TEST);
	for (unsigned int i = 0; i < chunkList.size(); i++) {
		chunkList[i].draw(projection, view);
		if (chunkList[i].doneLoadingFlag == true) {
			numTerrainChunksLoading--;
			if (chunkList[i].getChunkState() == 2){
				chunkList[i].genBuffers();
			}
			chunkList[i].doneLoadingFlag = false;
		}
	}
	return 1;
}

int TerrainSuperchunk::getNumLoadingTerrainChunks() {
	return numTerrainChunksLoading;
}

int TerrainSuperchunk::notifyFinishedLoadingOperation() {
	numTerrainChunksLoading--;
	return numTerrainChunksLoading;
}