/**************************************************************************************************
*
*	Textured_map.h
*
***************************************************************************************************/

#ifndef MISSION_CONTROL_TEXTURED_MAP_H_
#define MISSION_CONTROL_TEXTURED_MAP_H_

#include "stdHeaders.h"
#include "Class_Shader.h"
#include "Class_Text.h"

class MapTile {
private:
	std::string name;
	glm::vec2 pos;
	int width;
	int height;
	GLuint tileTexture;
	GLfloat *vertices;
	// Create Vertex Array Object
	GLuint tileVAO;
	GLuint tileVBO;
	GLuint tileEBO;
	Shader tileShader;
	unsigned int texture;
	int texState = -1; // Keeps track of tile texture loading. 0 is unloaded (blank tile), 1 is loaded, -1 is nothing
	//int textureLoadUnloadRequest = 0; // -1 for unload, +1 for load, 0 for nothing. Needed for multithread/asynchronous execution tracking
	unsigned int VBO, VAO, EBO;
	glm::mat4 translationMatrix;
	const GLchar* fragShader =
	{
		"#version 330 core\n"\
		"out vec4 FragColor;\n"\

		"in vec3 ourColor;\n"\
		"in vec2 TexCoord;\n"\

		// texture sampler
		"uniform sampler2D texture1;\n"\

		"void main()\n"\
		"{\n"\
			"FragColor = texture(texture1, TexCoord);\n"\
		"}\n"
	};
	const GLchar* vertShader =
	{
		"#version 330 core\n"\
		"layout(location = 0) in vec3 aPos;\n"\
		"layout(location = 1) in vec3 aColor;\n"\
		"layout(location = 2) in vec2 aTexCoord;\n"\
		"uniform mat4 model;\n"\
		"uniform mat4 view;\n"\
		"uniform mat4 projection;\n"\

		"out vec3 ourColor;\n"\
		"out vec2 TexCoord;\n"\

		"void main()\n"\
		"{\n"\
			"gl_Position = projection * view * model * vec4(aPos, 1.0);\n"\
			"ourColor = aColor;\n"\
			"TexCoord = vec2(aTexCoord.x, 1-aTexCoord.y);\n"\
		"}\n"
	};
public:
	MapTile();
	~MapTile();
	int genBlankTile(std::string tileName, glm::vec3 Pos, glm::vec3 colour);
	int loadTile();// std::string tileName, glm::vec3 Pos);
	int drawTile();
	int requestLoadTexture();
	int requestUnloadTexture();
	int handleTextureLoadUnloadRequests();
	glm::vec2 getPos();
	std::string getName();
	float getWidth();
	float getHeight();
	HANDLE threadHandle = NULL;
	DWORD threadID = -1;
	int isWaitingOnImageLoad = -1; //-1 for no (i.e. blank), 0 for loaded, +1 for loading
	static DWORD WINAPI asyncLoadImage(__in LPVOID lpParameter);
	int texWidth, texHeight;
	unsigned char* loadedSOILtex = NULL;
};

class TiledMap {
private:
	glm::vec2 bottomLeftCorner;
	std::vector<MapTile> tiles;
	std::vector<std::string> requiredMapTiles; //List of tiles required by the camera view. If the tile isn't on this list, it will be unloaded
	int numLoadingMapTiles;
public:
	TextRenderer mapTileFont;
	TiledMap();
	~TiledMap();
	int loadFonts();
	int create100kmTiledBlankOS();
	std::vector<std::string> findTilesInBoundingBox(glm::vec2 topLeft, glm::vec2 bottomRight, int spacing);
	int setRequiredTiles(std::vector<std::string> requiredTiles);
	int chunkLoadUnloadMapTiles();
	int draw();
};

#endif // !MISSION_CONTROL_TEXTURED_MAP_H_
