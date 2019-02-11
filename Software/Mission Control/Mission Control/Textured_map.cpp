/**************************************************************************************************
*
*	Textured_map.cpp
*
*	This file contains the implementations of the map tile class, and the tiled map class.
*	These classes implement the loading of .tiff map textures downloaded from the OS website.
*
*	Changelog
*	24.08.2018, Peter - re-wrote and improved the map tile chunking algorithm, cleared up some
*	of the appalling memory management...
*
***************************************************************************************************/

#include "stdHeaders.h"
#include "Textured_map.h"
#include <filesystem>
#include "MapWindow.h"

/******************************************************************************
*	Map Tile class
*	Loads and stores an individual map tile and its location on the map
*******************************************************************************/

MapTile::MapTile() {
}

MapTile::~MapTile() {

}


int MapTile::genBlankTile(std::string tileName, glm::vec3 Pos, glm::vec3 colour){
	//destroy any previous textures that may have been loaded...
	glDeleteTextures(1, &texture);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	tileShader.deleteShader();
	tileShader.~Shader();

	name = tileName;
	pos = Pos;

	// build and compile our shader zprogram
	tileShader = Shader(vertShader, fragShader);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	width = 100000;
	height = 100000;
	float vertices[] = {
		// positions		   // texture coords
		float(width),  float(height), 0.0f,   1.0f, 1.0f, // top right
		float(width),  0.0000000000f, 0.0f,   1.0f, 0.0f, // bottom right
		0.000000000f,  0.0000000000f, 0.0f,   0.0f, 0.0f, // bottom left
		0.000000000f,  float(height), 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);


	// The stuff we're going to render to
	GLuint frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glGenTextures(1, &texture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 500.0f, 500.0f, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	//Did it work? Is our baby framebuffer ok?
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "BLANK MAP TILE ERROR: FRAMEBUFFER NOT OK (Also potential memory leak =] )" << std::endl;
		return 0;
	}

	//Render the name of the tile, followed by "loading..." to a texture
	// Render to our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	tileShader.use();
	translationMatrix = glm::mat4(1.0);
	translationMatrix = glm::translate(translationMatrix, Pos);
	tileShader.setMat4("model", translationMatrix);
	glClearColor(colour.r, colour.g, colour.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	OSExplorerMap.mapTileFont.RenderText("Loading...",30, 30, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	glDeleteFramebuffers(1, &frameBuffer); //delete!
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //Unbind
	return 1;
}

int MapTile::loadTile(){//std::string tileName, glm::vec3 Pos) {
	if (!loadedSOILtex || loadedSOILtex == NULL) {
		std::cout << "ERROR: failed to access loaded map texture " << name.c_str() << std::endl;
		return 0;
	}
	//destroy any previous textures that may have been loaded...
	glDeleteTextures(1, &texture);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	tileShader.deleteShader();
	tileShader.~Shader();

	//name = tileName;
	//pos = Pos;
	// build and compile our shader zprogram
	tileShader = Shader(vertShader, fragShader);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	width = 100000;
	height = 100000;
	float vertices[] = {
		// positions				   // texture coords
		float(width),  float(height), 0.0f,   1.0f, 1.0f, // top right
		float(width),  0.0000000f, 0.0f,   1.0f, 0.0f, // bottom right
		0.000000000f,  0.0000000f, 0.0f,   0.0f, 0.0f, // bottom left
		0.000000000f,  float(height), 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// load and create a texture 
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps

	GLenum ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR) {
		std::cout << "TILE LOAD SETUP GL_ERROR: " << ErrorCheckValue << std::endl;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, loadedSOILtex);

	SOIL_free_image_data(loadedSOILtex);
	glGenerateMipmap(GL_TEXTURE_2D);
	//Now... where should we stick it?
	tileShader.use();
	
	translationMatrix = glm::mat4(1.0f);
	glm::vec3 localOrigin = glm::vec3(pos.x, pos.y, 0.0f);
	translationMatrix  = glm::translate(translationMatrix, localOrigin);
	tileShader.setMat4("model", translationMatrix);
	
	return 1;
}

int MapTile::requestLoadTexture() {
	int oldVal = isWaitingOnImageLoad;
	int isRequestSuccessful = 0;
	if (threadHandle == NULL && threadID == -1) {
		if (isWaitingOnImageLoad == -1 && (texState == -1 || texState == 0)) {
			loadedSOILtex = NULL;
			std::cout << "Requesting tile load " << name << std::endl;
			isWaitingOnImageLoad = +1; // Set this to 1 before we check to avoid race condition
			threadHandle = CreateThread(0, 0, asyncLoadImage, this, 0, &threadID);
			if (threadHandle != NULL) { //did we really create a thread?
				isRequestSuccessful = 1;
			}
			else {
				isWaitingOnImageLoad = oldVal;
				std::cout << "ERROR: Failed to create tile loading thread!" << std::endl;
			}
		}
	}
	return isRequestSuccessful;
}

int MapTile::requestUnloadTexture() {
	int oldVal = isWaitingOnImageLoad;
	if ((isWaitingOnImageLoad == 0 || isWaitingOnImageLoad == -1) && (texState == -1 || texState == 1)) {
		std::cout << "Unloading map tile " << name << "...";
		genBlankTile(name, glm::vec3(pos.x, pos.y, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		texState = 0;
		isWaitingOnImageLoad = -1;
		std::cout << "done!" << std::endl;
	}
	return oldVal;
}

DWORD WINAPI MapTile::asyncLoadImage(__in LPVOID lpParameter){
	MapTile *thisTile = (MapTile*)lpParameter;
	std::cout << "Loading Tile: " << thisTile->name << std::endl;
	thisTile->loadedSOILtex = NULL;
	std::string file = "data/maps/OS/District Geotiff/png/" + thisTile->name + ".png";
	unsigned char* image = SOIL_load_image(file.c_str(), &(thisTile->texWidth), &(thisTile->texHeight), 0, SOIL_LOAD_RGB);
	if (!image) {
		std::cout << "ERROR: SOIL failed to asynchronously load map texture for " << thisTile->name.c_str() << std::endl;
		thisTile->isWaitingOnImageLoad = -1;
		thisTile->loadedSOILtex = NULL;
		return 0;
	}
	thisTile->loadedSOILtex = image;
	thisTile->isWaitingOnImageLoad = 0;
	return 1;
}

int MapTile::handleTextureLoadUnloadRequests() {
	//See what new load/unload threads need creating
	if (threadHandle != NULL && threadID != -1) {
		DWORD result = WaitForSingleObject(threadHandle, 0);
		if (result == WAIT_OBJECT_0) { //if it has finished...
			if (isWaitingOnImageLoad == 0) { //Check we still want it, and/or the SOIL image load worked
				loadTile();
			}
			threadHandle = NULL;
			threadID = -1;
			return 2; //say we're done loading this tile one way or another
		}
	}
	return 1;
}

std::string MapTile::getName() {
	return name;
}
glm::vec2 MapTile::getPos() {
	return pos;
}
float MapTile::getWidth() {
	return float(width);
}
float MapTile::getHeight() {
	return float(height);
}

int MapTile::drawTile() {
	// bind Texture
	glBindTexture(GL_TEXTURE_2D, texture);
	// render container
	tileShader.use();
	mapCamera.setAspectRatio(mapWindowHeight / mapWindowWidth);
	tileShader.setMat4("view", mapCamera.getViewMatrix());
	tileShader.setMat4("projection", mapCamera.getProjectionMatrix());
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	return 1;
}

/******************************************************************************
*	Tiled Map class
*	Loads and stores a list of map tiles to create a complete OS map
*******************************************************************************/

TiledMap::TiledMap() {
}

TiledMap::~TiledMap() {
}

int TiledMap::loadFonts() {
	mapTileFont = TextRenderer(500, 500);
	mapTileFont.Load("data/Fonts/arial.ttf", 12);
	return 1;
}

int TiledMap::create100kmTiledBlankOS() {
	for(int E = 0; E < 7; E++){
		for (int N = 0; N < 13; N++) {
			glm::vec2 eastingNorthing(E * 100 * 1000, N * 100 * 1000);
			std::string tileName = util_EastingNorthingToGridLetter(eastingNorthing);
			MapTile newTile;
			newTile.genBlankTile(tileName, glm::vec3 (eastingNorthing.x, eastingNorthing.y, 0.0), glm::vec3(0.0f, 0.0f, 1.0f));
			tiles.push_back(newTile);
		}
	}
	return 1;
}

std::vector <std::string> TiledMap::findTilesInBoundingBox(glm::vec2 topLeft, glm::vec2 bottomRight, int spacing) { // Spacing is tile width in m (usually 100,000m)
	std::vector<std::string> result;
	for (float e = topLeft.x; e <= bottomRight.x; e += spacing) {
		for (float n = bottomRight.y; n <= topLeft.y; n += spacing) {
			std::string gridSquare = util_EastingNorthingToGridLetter(glm::vec2(e, n), false);
			result.emplace_back(gridSquare);
		}
	}
	return result;
}

int TiledMap::setRequiredTiles(std::vector<std::string> requiredTiles) {
	requiredMapTiles = requiredTiles;
	for (unsigned int i = 0; i < tiles.size(); i++) {
		bool tileFound = false;
		for (unsigned int j = 0; j < requiredMapTiles.size() && numLoadingMapTiles <= 2; j++) {
			if (tiles[i].getName() == requiredMapTiles[j]) {
				if (tiles[i].requestLoadTexture()) {
					numLoadingMapTiles++;
				}
				tileFound = true;
				break;
			}
		}
		if (!tileFound) {
			tiles[i].requestUnloadTexture();
		}
	}
	return 1;
}

int TiledMap::chunkLoadUnloadMapTiles() {
	//Check threads for existence/completion
	for (unsigned int i = 0; i < tiles.size(); i++) {
		if (tiles[i].handleTextureLoadUnloadRequests() == 2) { //it's done!
			numLoadingMapTiles--;
		}
	}
	return 1;
}

int TiledMap::draw() {
	for(int i = tiles.size()-1; i >= 0; i--){
		tiles[i].drawTile();
	}
	return 1;
}