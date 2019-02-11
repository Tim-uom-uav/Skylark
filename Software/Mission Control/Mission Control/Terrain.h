/**********************************************************************************************************************
*
*	Terrain.h
*	Class for loading the .asc terrain pointclouds, parsing them and bundling them in to a format that is useful to
*	OpenGL, and rendering them.
*
***********************************************************************************************************************/

#ifndef MISSION_CONTROL_TERRAIN_H_
#define MISSION_CONTROL_TERRAIN_H_

#include "stdHeaders.h"
#include "Class_Shader.h"
#include "Class_Camera.h"
#include "FlightWindow.h"

/*************************************************************************
*	Terrain Chunk Class
*	Loads in .asc files
*	Stores the high-resolution chunk for each block of terrain
*	Responsible for supersampling for LOD models
**************************************************************************/
class TerrainChunk {
private:
	typedef struct
	{
		float XYZW[4];
		float RGBA[4];
	} Vertex;
	std::string path;
	std::string name;
	glm::vec3 pos;
	std::vector<std::vector<int>> heightMapPointCloud;
	std::vector<Vertex> heightmapVertices;
	std::vector<GLuint> heightmapSurfaceIndices;
	double lowerLeftCornerLat, lowerLeftCornerLong;
	double lateralPointSpacing;
	double longitudinalPointSpacing;

	int chunkState = 0; // 0 = empty, 1 = loading, 2 = loaded, 3 = nonexistant

	//Graphics stuff
	Shader terrainShader;
	GLuint
		VertexShaderId,
		FragmentShaderId,
		ProgramId,
		VaoId,
		BufferId,
		IndexBufferId;
	const GLchar* TerrainVertexShader =
	{
		"#version 400\n"\

		"layout(location=0) in vec4 in_Position;\n"\
		"layout(location=1) in vec4 in_Color;\n"\
		"out vec4 ex_Color;\n"\
		"uniform mat4 model;\n"\
		"uniform mat4 view;\n"\
		"uniform mat4 projection;\n"\

		"void main(void)\n"\
		"{\n"\
		"  gl_Position = projection * view * model * in_Position;\n"\
		"  ex_Color = in_Color;\n"\
		"}\n"
	};
	const GLchar* TerrainFragmentShader =
	{
		"#version 400\n"\

		"in vec4 ex_Color;\n"\
		"out vec4 out_Color;\n"\

		"void main(void)\n"\
		"{\n"\
		"  out_Color = ex_Color;\n"\
		"}\n"
	};
	static DWORD WINAPI asyncLoadFile(__in LPVOID lpParameter);
public:
	HANDLE threadHandle = NULL;
	DWORD threadID = -1;
	bool doneLoadingFlag = false; //true if done loading but the superchunk parent hasn't noticed yet
public:
	TerrainChunk();
	~TerrainChunk();
	std::string getName();
	std::string setName(std::string newName);
	glm::vec3 setPos(glm::vec3 newPos);
	int reqLoadUnloadChunk(bool load, int numTerrainChunksCurrentlyLoading, std::string reason);
	int getChunkState(); // 0 = empty, 1 = loading, 2 = loaded, 3 = nonexistant file
	int setChunkState(int newState); // 0 = empty, 1 = loading, 2 = loaded, 3 = nonexistant file
	glm::vec2 getTopLeftCorner();
	glm::vec2 getBottomRightCorner();
	int loadFile(std::string filePath);
	int startAsyncLoadFile();
	int genSurfaceVertices();
	int genSurfaceIndices();
	int genSupersampledSurface(int resolution);
	int unload();
	int draw(glm::mat4 projection, glm::mat4 view);
	int genBuffers();
	double getLowerLeftLat();
	double getLowerLeftLong();
	double getLateralSpacing();
	double getLongitudinalSpacing();
};


/*************************************************************************
*	Terrain Superchunk class
*	Stores the largest terrain sections - one for each 100km square
**************************************************************************/
class TerrainSuperchunk {
private:
	int numTerrainChunksLoading = 0;
	glm::vec2 pos; //lower left corner
	glm::vec2 dimensions;
	std::string name;
public:
	std::vector<TerrainChunk> chunkList;
	TerrainSuperchunk();
	~TerrainSuperchunk();
	std::string getName();
	int initSuperchunk(std::string gridSquare, glm::vec2 position); //Init 100km gridsquare with 100 10km squares
	int manageTerrainChunks(glm::vec2 cameraPos, float viewRadius);
	int draw(glm::mat4 projection, glm::mat4 view);
	int getNumLoadingTerrainChunks();
	int notifyFinishedLoadingOperation();
};

#endif // !MISSION_CONTROL_TERRAIN_H_
