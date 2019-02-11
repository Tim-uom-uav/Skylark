/**************************************************************************************************
*
*	MapWindow.cpp
*
*	DataWindow.cpp has top-level control of the data window - it controls general flow, drawing, call order, etc.
*	
*	Map Window detailed spec (yet more detail to be found in relevant files):
*	
*	
***************************************************************************************************/

#include "stdHeaders.h"
#include "General_OpenGL_Prototypes.h"
#include "MapWindow.h"
#include "Textured_map.h"
#include "Variables.h"
#include "Weather_Data.h"

GLFWwindow* mapWindow = 0;
TiledMap OSExplorerMap;
OrthoCamera mapCamera;
TextRenderer mapWindowFont;

double mapWindowMouseX = 0;
double mapWindowMouseY = 0;
double mapWindowLastMouseX = 0;
double mapWindowLastMouseY = 0;
double mapWindowMouseDown = false;

float mapWindowWidth;
float mapWindowHeight;
int getMapWindowInputs();
void mapWindow_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mapWindow_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

int setupMapWindow() {
	//Setup the map window OGL context...
	mapWindowWidth = float(DISPLAY_WIDTH / 2);
	mapWindowHeight = float((DISPLAY_HEIGHT - 90) / 2);
	mapWindow = glfwCreateWindow(int(mapWindowWidth), int(mapWindowHeight), "Map Window", NULL, NULL);
	if (mapWindow == NULL) {
		std::cout << "Failed to create GLFW map window" << std::endl;
		glfwTerminate();
		return 0;
	}
	glfwHideWindow(mapWindow);
	glfwSetWindowPos(mapWindow, DISPLAY_WIDTH / 2, 40);
	glfwMakeContextCurrent(mapWindow);
	glfwSetFramebufferSizeCallback(mapWindow, framebuffer_size_callback);

	glfwSetInputMode(mapWindow, GLFW_STICKY_MOUSE_BUTTONS, 1);
	glfwSetScrollCallback(mapWindow, mapWindow_scroll_callback);
	glfwSetMouseButtonCallback(mapWindow, mapWindow_mouse_button_callback);
	
	return 1;
}

int loadMapWindowData() {
	glfwMakeContextCurrent(mapWindow);

	mapWindowFont = TextRenderer(mapWindowWidth, mapWindowHeight);
	mapWindowFont.Load("data/Fonts/arial.ttf", 12);

	OSExplorerMap.loadFonts();
	OSExplorerMap.create100kmTiledBlankOS();
	return 1;
}

int drawMapWindow() {
	glfwMakeContextCurrent(mapWindow);
	processInput(mapWindow);
	glClearColor(0.894,0.941,0.996, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/****************************************************************
	*	Draw OS map PNG tiles!
	*****************************************************************/
	glDisable(GL_DEPTH_TEST);
	OSExplorerMap.draw();

	/***************************************************************
	*	Draw map objects!
	****************************************************************/
	for (int i = 0; i < g_ADSBaircraftList.size(); i++) {
		g_ADSBaircraftList[i].drawToMapWindow(mapCamera.getViewMatrix(), mapCamera.getProjectionMatrix());
	}
	g_Glider.drawMapIcon(mapCamera.getViewMatrix(), mapCamera.getProjectionMatrix());

	/****************************************************************
	*	Draw Text GUI
	*****************************************************************/
	std::string viewPositionString = "E: " + std::to_string(int(mapCamera.getPosition().x)) + " N: " + std::to_string(int(mapCamera.getPosition().y))+". GridRef "+util_EastingNorthingToGridLetter(mapCamera.getPosition());
	mapWindowFont.RenderText(viewPositionString, 5.0f, 5.0f, 1.0f, glm::vec3(0.0f,0.0f,0.0f));
	std::string windSpeedString = "Wind Speed: " + std::to_string(getWindSpeedWithEastingNorthing(mapCamera.getPosition())) + "m/s";
	mapWindowFont.RenderText(windSpeedString, 5.0f, 17.0f, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	std::string widthString;
	if (mapCamera.getDisplayWidth() <= 2000) {
		widthString = std::to_string(int(mapCamera.getDisplayWidth())) + "m";
	}
	else if (mapCamera.getDisplayWidth() < 10000) {
		widthString = std::to_string(mapCamera.getDisplayWidth()/1000);
		widthString = widthString.substr(0, 3) + "km";
	}
	else {
		widthString = std::to_string(int(mapCamera.getDisplayWidth()/1000)) + "km";
	}
	mapWindowFont.RenderText(widthString, mapWindowWidth - 50, mapWindowHeight - 15, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));

	/****************************************************************
	*	Misc/Housekeeping
	*****************************************************************/
	glfwSwapBuffers(mapWindow);
	getMapWindowInputs();

	/****************************************************************
	*	Resource handling!
	*	Right! How far can we see, at what detail, so what resources
	*	do we need to request?
	*****************************************************************/
	//Start by calculating our viewing extents
	double viewWidth = mapCamera.getDisplayWidth();
	double viewHeight = viewWidth*mapCamera.getAspectRatio();
	glm::vec2 viewPos = mapCamera.getPosition();
	//Iterate through all the grid squares to determine which are within field of view
	std::vector<std::string> reqMapTiles = OSExplorerMap.findTilesInBoundingBox(glm::vec2(viewPos.x - (viewWidth / 2.0), viewPos.y + (viewHeight / 2.0)), glm::vec2(viewPos.x + (viewWidth / 2.0), viewPos.y - (viewHeight / 2.0)), 100000);
	OSExplorerMap.setRequiredTiles(reqMapTiles);
	//Now demand them! Unless we're viewing them at less than 1/10 size, then we just use the really low res one-piece map!
	OSExplorerMap.chunkLoadUnloadMapTiles();
	return 1;
}

int getMapWindowInputs() {
	mapWindowLastMouseX = mapWindowMouseX;
	mapWindowLastMouseY = mapWindowMouseY;
	glfwGetCursorPos(mapWindow, &mapWindowMouseX, &mapWindowMouseY);
	//Click-and-drag
	if (mapWindowMouseDown) {
		mapCamera.move(glm::vec3(mapWindowLastMouseX - mapWindowMouseX, mapWindowMouseY - mapWindowLastMouseY, 0)*1000.0f);
	}
	return 1;
}

void mapWindow_scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	if (abs(yoffset) > 0.3) {
		yoffset = 0.3*(yoffset / abs(yoffset)); //clamp to 0.3, but preserve the sign
	}
	mapCamera.zoom(float(abs(1 + yoffset)));
}

void mapWindow_mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		mapWindowMouseDown = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		mapWindowMouseDown = false;
	}
}