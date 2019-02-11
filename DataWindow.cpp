/********************************************************************************************************************************
*	
*	DataWindow.cpp
*
*	DataWindow.cpp has top-level control of the data window - it controls general flow, drawing, call order, etc.
*	
*	Data Window detailed spec (yet more detail to be found in relevant files):
*	-> Display raw program debug data as lifted off error handling and notification stacks
*	-> Display raw aircraft telemetry (i.e. received telemetry raw strings)
*	-> Display each telemetry category as text
*
*	
*	Changelog:
*	06.07.2018, Peter Naylor - created
*	19.08.2018, Peter Naylor - spec updated and fleshed out
*	23.08.2018, Peter Naylor - added various bits of overhead and a framerate counter
*
*********************************************************************************************************************************/

#include "stdHeaders.h"
#include "General_OpenGL_Prototypes.h"
#include "DataWindow.h"
#include "Class_Text.h"
#include "Variables.h"

GLFWwindow* dataWindow = 0;
TextRenderer dataWindowFont;

float dataWindowWidth;
float dataWindowHeight;

int setupDataWindow() {
	dataWindowWidth = DISPLAY_WIDTH/2.0f;
	dataWindowHeight = (DISPLAY_HEIGHT - 90) / 2.0f;
	dataWindow = glfwCreateWindow(int(dataWindowWidth), int(dataWindowHeight), "Data Window", NULL, NULL);
	if (dataWindow == NULL) {
		std::cout << "Failed to create GLFW Data window" << std::endl;
		glfwTerminate();
		return 0;
	}
	glfwHideWindow(dataWindow);
	glfwSetWindowPos(dataWindow, DISPLAY_WIDTH / 2, 80 + ((DISPLAY_HEIGHT - 90) / 2));
	glfwMakeContextCurrent(dataWindow);
	glfwSetFramebufferSizeCallback(dataWindow, framebuffer_size_callback);

	return 1;
}

int loadDataWindowData() {
	glfwMakeContextCurrent(dataWindow);
	dataWindowFont = TextRenderer((GLuint)dataWindowWidth, (GLuint)dataWindowHeight);
	dataWindowFont.Load("data/Fonts/arial.ttf", 12);
	return 1;
}

int drawDataWindow() {
	glfwMakeContextCurrent(dataWindow);
	processInput(dataWindow);
	glClearColor(0.4f, 0.1f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	dataWindowFont.RenderText("FPS: " + std::to_string(int(g_FPS)), 8.0f, 15.0f, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));

	glfwSwapBuffers(dataWindow);
	glfwPollEvents();
	return 1;
}