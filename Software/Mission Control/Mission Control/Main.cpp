/****************************************************************************************************************************************************
*
*	Main.cpp
*	Mission Control program entry point, all functions are called from here. DO NOT write code directly into here unless it can possibly be avoided,
*	it should be called by a function either here or elsewhere. The purpose of this file is to make the program flow as obvious and
*	complication-free as possible!
*	
*	Mission Control Project created by Peter Naylor, 06/07/2018
*	Last Modified by Peter Naylor 16/07/2018
*
*	About Mission Control:
*	-> Mission control is the ground control software "brain" for controlling the glider safely. Thus it should exactly what is necessary to ensure
*	operation of the glider - no less and NO MORE. Feature creep means complexity creep which means bug creep, which means LOM creep.
*	-> We are operating at too low a telemetry transmission power to receive live video, therefore we shall need to fly by artificial horizon.
*	-> Lack of vision also means lack of situational awareness, and therefore no chance of collision avoidance or navigation.
*	-> Therefore Mission Control provides the pilot with a live artificial horizon to fly by, plus simulated terrain from OS and USGS heightmaps to
*	give a "pilot's-eye" view. GPS co-ordinates from the aircraft are used to give the correct lat/long/alt. A 2D map view is also available.
*	-> To allow safe navigation (since signal will be lost if we actually end up in close proximity to terrain), met data for wind, cloud and
*	precipitation is displayed for navigation at altitude.
*	-> City/Town/Village extents will be plotted to ensure we do not overfly
*	-> To ensure collision avoidance, live aircraft transponder data is pulled from the ADS-B exchange and displayed. To be clear: this does not
*	allow us to run the gauntlet of air lanes, but it does give us a good idea of where to avoid if (a) we aren't where we should be or (b) an
*	aircraft is not where it should be or is expected to be (think a C-172 pilot on his sunday morning joyride)
*	-> If we end up having to operate over the sea, a similar functionality for shipping might not be a bad idea.
*
*	Design Philosophy:
*	-> KISS - This is to make the pilot's job as easy as possible, not increase his workload
*	-> Graphical Data is processed and understood by the human brain much faster than words and numbers. Therefore, as much data as appropriate
*	should be displayed graphically in nature to increase the data fed to the pilot. Precise numerical values should also be available if the pilot
*	or someone else in the tent has their interest piqued
*	-> Fault tolerance. Every data value should be treated as suspect, particularly those downlinked from the plane, or over the internet. Dumb
*	values should be handled by the program, and where appropriate, a *non-blocking* warning should be sent out. Under no circumstances should
*	out-of-range values cause a software crash or malfunction. This means every loaded file should also be treated as garbage, failure to load, etc.
*
*	Programming standards/consistency/guide to unmaintainable code mitigation
*	-> All global variables to be prefixed with "g_"
*	-> Util functions to be prefixed with "util_"
*	-> THE NEXT PERSON TO USE "using namespace [...]" WILL BE SHOT
*	-> CamelCase rules unless there is a very good reason (specified exception, meaning would be totally lost (e.g. m/s = m_s =/= milliseconds)
*	-> Otherwise non-returning functions to be "int", not "void". Return 1 when successful, 0 when not, and other things when necessary.
*
*	Mission Control High-Level Spec (Always a good idea, and will bring us at least into the spirit of DO278):
*	-> Three windows, each capable of running on a different screen - Flight, Map and Data
*	-> Flight Window:
*		-> Locked to position and rotation of aircraft in flight (w/lookaround)
*		-> Display 3D terrain of the UK
*		-> Terrain to be textured with options for height, satellite view, terrain, OS Landranger, public-access land, etc
*		-> Display an artificial horizon w/velocity vectors, headings etc
*		-> Display aircraft positions as pulled from the ADS-B exchange, with nametag, speed, distance and altitude
*		-> Display no-fly areas
*		-> Display weather, including wind, precipitation, turbulence predictions
*		-> Show launch/landing zones, optimum flight path, predicted flightpath, waypoints, etc.
*	-> Map Window:
*		-> Locked to Lat/Long position of aircraft
*		-> Displays OS Landranger UK map
*		-> Shows aircraft 2D track
*		-> Shows Launch/landing sites
*		-> Shows no-fly zones
*		-> Shows aircraft positions as pulled from ADS-B
*	-> Data Window
*		-> Tasked with displaying the general health of the aircraft, data flight data and so-forth
*		-> Generates a flight log streamed to the harddrive in case of software crash and for future reference
*		-> Most data likely to be numeric, but make it graphical where useful. This usually means graphs.
*		-> Where appropriate, values should be highlighted green for ok, orange for suspect, and red for malfunction/out-of-family etc.
*
*	More Detailed information/spec is to be found in each's relevant C++ files.
*
*	Main.cpp Changelog:
*		06.07.2018, Peter Naylor - Created
*		19.08.2018, Peter Naylor - Spec updated & fleshed out
*		23.08.2018, Peter Naylor - Added in some Data Window overhead and a framerate counter
*
*****************************************************************************************************************************************************/


#include "stdHeaders.h"
#include "General_OpenGL_Prototypes.h"

#include "Main.h"
#include "FlightWindow.h"
#include "MapWindow.h"
#include "DataWindow.h"
#include "Variables.h"
#include "HTTPS_Get.h"

long long lastFrameTime = 0;

int main(){
	unsigned int n = std::thread::hardware_concurrency();
	std::cout << n << " concurrent threads are supported.\n";
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	lastFrameTime = ms.count();
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
	DISPLAY_WIDTH = mode->width;
	DISPLAY_HEIGHT = mode->height;

	setupFlightWindow();
	setupMapWindow();
	setupDataWindow();
	//Setup GLAD
	glfwMakeContextCurrent(mapWindow);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Load maps, textures, etc etc
	if (importMapVertexData()) {
		std::cout << "Successfully loaded heightmap data! (Praise be to NASA!)" << std::endl;
	}
	else {
		std::cout << "ERROR: Failed to load heightmap data" << std::endl;
	}

	//Load more data...
	loadMapWindowData();
	loadDataWindowData();
	loadFlightWindowData();

	//Show windows!
	glfwShowWindow(dataWindow);
	glfwShowWindow(flightWindow);
	glfwShowWindow(mapWindow);

	glfwMakeContextCurrent(mapWindow);
	g_Glider.setupMapIcon();
	std::string apiSearchString = "/VirtualRadar/AircraftList.json";
	apiSearchString += "?lat=" + std::to_string(50.72);
	apiSearchString += "&lng=" + std::to_string(-3.53);
	apiSearchString += "&fDstL=0&fDstU=50";
	g_ADSBaircraftList = extractAircraftFromJSON(https_get("public-api.adsbexchange.com", "/VirtualRadar/AircraftList.json?lat=50.72&lng=-3.53&fDstL=0&fDstU=100").str());
	std::cout << "Found " << g_ADSBaircraftList.size() << " aircraft!" << std::endl;
		
	// render loop
	while (!glfwWindowShouldClose(mapWindow)){
		//draw windows and poll events for each
		drawFlightWindow();
		drawMapWindow();
		drawDataWindow();
		// Calculate framerate
		ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		g_FPS = 1000.0f / (ms.count() - lastFrameTime);
		lastFrameTime = ms.count();
	}

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window){
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	if (width > 0 && height > 0) {
		if (window == flightWindow) {
			glfwMakeContextCurrent(flightWindow);
			flightWindowWidth = width;
			flightWindowHeight = height;
			glViewport(0, 0, flightWindowWidth, flightWindowHeight);
			flightWindowFont.resize(GLuint(flightWindowWidth), GLuint(flightWindowHeight));
		}
		else if (window == mapWindow) {
			glfwMakeContextCurrent(mapWindow);
			mapWindowWidth = float(width);
			mapWindowHeight = float(height);
			glViewport(0, 0, mapWindowWidth, mapWindowHeight);
			mapWindowFont.resize(GLuint(mapWindowWidth), GLuint(mapWindowHeight));
		}
		else if (window == dataWindow) {
			glfwMakeContextCurrent(dataWindow);
			dataWindowWidth = float(width);
			dataWindowHeight = float(height);
			glViewport(0, 0, dataWindowWidth, dataWindowHeight);
			mapWindowFont.resize(GLuint(dataWindowWidth), GLuint(dataWindowHeight));
		}
	}
}