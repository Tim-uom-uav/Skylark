/********************************************************************************************************************************
*	
*	FlightWindow.cpp
*	
*	FlightWindow.cpp has top-level control of the flight window - it controls general flow, drawing, call order, etc.
*
*	Flight Window detailed spec (yet more detail to be found in relevant files):
*	-> Display a 3D map of the terrain to varying levels of detail & texturing
*		-> Use the OS 50m resolution maps chunked out to a 100km square from the camera regardless of altitude
*		-> Use much lower resolution map out to the edge of the National Grid, with 100km square hole in for OS.
*		-> Use USGS 5m resolution maps when within 10,000ft of the ground for the nearest 4 10km OS squares
*		-> Several texturing options available:
*			-> Pure heightmap (by grayscale or colour?)
*			-> Pure satellite view
*			-> OS Landranger 250,000 view
*			-> Public Access land map
*	-> Display aircraft transponder locations
*		-> Updated at maximum frequency (ADS-B updates at ~30 second mark once per minute)
*		-> All aircraft within 200km radius
*		-> Display past 200km of track
*		-> Display last reported location as point on track
*		-> Display predicted current location with nametag, altitude, speed, and distance (with +/- sign to indicate closure)
*		-> Different icon for type - airliner, military, light aircraft, helicopter, balloon, glider, etc.
*	-> Display cloud cover
*		-> Should be on/off/fade controllable
*		-> Gives an approximate idea of where clouds are and their type so they can be avoided
*		-> Possibility to download low-res image from aircraft on descent to refine knowledge?
*	-> Display waypoints
*		-> Display launch and landing points
*		-> Display optimal glidepath waypoints (accounting for wind) to reach landing zone safely from current position
*	-> HUD
*		-> Artifical Horizon
*		-> View bearing
*		-> Airspeed
*		-> Groundspeed
*		-> Wind bearing
*		-> Heading
*
*	Changelog:
*		06.07.2018, Peter Naylor - created.
*		19.07.2018, Peter Naylor - spec updated and fleshed out
*		22.07.2018, Peter Naylor - terrain chunking improved
*		25.09.2018, Peter Naylor - Terrain chunking rewritten
*		04.10.2018, Peter Naylor - Terrain Chunking rewritten again. Now almost completely handled by TerrainSuperChunk class.
*
*********************************************************************************************************************************/

#include "stdHeaders.h"
#include "General_OpenGL_Prototypes.h"
#include "FlightWindow.h"
#include "Class_Shader.h"
#include "Class_Camera.h"
#include "Terrain.h"
#include "Variables.h"
#include "FlightWindowHUD.h"
#include "MapWindow.h"

GLFWwindow* flightWindow = 0;
unsigned int flightWindowWidth;
unsigned int flightWindowHeight;
Camera flightWindowCamera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0, 0);
double flightWindowMouseX = 0;
double flightWindowMouseY = 0;
TextRenderer flightWindowFont;
TextRenderer flightWindowHUDfont;

int setupFlightWindow() {
	flightWindowWidth = DISPLAY_WIDTH / 2;
	flightWindowHeight = DISPLAY_HEIGHT - 90;
	flightWindow = glfwCreateWindow(flightWindowWidth, flightWindowHeight, "Flight Window", NULL, NULL);
	if (flightWindow == NULL) {
		std::cout << "Failed to create GLFW Flight window" << std::endl;
		glfwTerminate();
		return 0;
	}
	glfwHideWindow(flightWindow);
	glfwMakeContextCurrent(flightWindow);
	glfwSetWindowPos(flightWindow, 0, 40);
	glfwSetFramebufferSizeCallback(flightWindow, framebuffer_size_callback);
	return 1;
}

int loadFlightWindowData() {
	glfwMakeContextCurrent(flightWindow);
	flightWindowFont = TextRenderer(flightWindowWidth, flightWindowHeight);
	flightWindowFont.Load("data/Fonts/arial.ttf", 12);

	flightWindowHUDfont = TextRenderer(500, 500);
	flightWindowHUDfont.Load("data/Fonts/arial.ttf", 15);

	setupFlightWindowHUD();
	return 1;
}

int drawFlightWindow() {
	glfwMakeContextCurrent(flightWindow);
	processInput(flightWindow);
	getFlightWindowInputs();
	glClearColor(0.0f, 0.75f, 0.75f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glm::mat4 projection = glm::perspective(glm::radians(50.0f), (float)flightWindowWidth / (float)flightWindowHeight, 0.01f, 500000.0f); //set 500km far clip plane
	flightWindowCamera.setMode(flightWindowCamera.CAMERA_MODE_ANGLE);
	flightWindowCamera.setPosition(glm::vec3(mapCamera.getPosition().x, mapCamera.getPosition().y, 5000));
	flightWindowCamera.setAngle(float(flightWindowMouseY), float(flightWindowMouseX), 0);

	//update relevant glider variables
	g_Glider.setPos(glm::vec4(flightWindowCamera.getPosition().x, flightWindowCamera.getPosition().y, flightWindowCamera.getPosition().z, 0));
	g_Glider.setFlightWindowViewDirection(glm::vec3(cos(glm::radians(double(flightWindowMouseX)))*cos(glm::radians(double(flightWindowMouseY))), sin(glm::radians(double(flightWindowMouseX)))*cos(glm::radians(double(flightWindowMouseY))), 0));

	glm::mat4 view = flightWindowCamera.GetViewMatrix();
	for (unsigned int i = 0; i < g_terrain.size(); i++) {
		g_terrain[i].draw(projection, view);
	}

	for (unsigned int i = 0; i < g_ADSBaircraftList.size(); i++) {
		g_ADSBaircraftList[i].drawToFlightWindow(view, projection);
	}
	
	drawFlightWindowHUD();

	glfwSwapBuffers(flightWindow);

	/****************************************************************
	*	Resource Handling!
	*****************************************************************/
	//We want to give a list of all the tiles we want - those already there will be kept, the others deleted!
	glm::vec2 cameraPos = glm::vec2(flightWindowCamera.getPosition().x, flightWindowCamera.getPosition().y);

	//The new version of this algorithm takes the current camera position, and then checks if any of a superchunk is within a radius of it
	for (unsigned int i = 0; i < g_terrain.size(); i++) {
		g_terrain[i].manageTerrainChunks(cameraPos, 60000); // view everything within an 60km radius
	}
	return 1;
}

int getFlightWindowInputs() {
	glfwGetCursorPos(flightWindow, &flightWindowMouseX, &flightWindowMouseY);
	return 1;
}