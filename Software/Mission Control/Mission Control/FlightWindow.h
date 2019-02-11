#ifndef MISSION_CONTROL_FLIGHT_WINDOW_H_
#define MISSION_CONTROL_FLIGHT_WINDOW_H_

#include "Class_Text.h"
#include "Class_Camera.h"

extern GLFWwindow* flightWindow;
extern Camera flightWindowCamera;
extern unsigned int flightWindowWidth;
extern unsigned int flightWindowHeight;
int getFlightWindowInputs();
int loadFlightWindowData();

extern double flightWindowMouseX;
extern double flightWindowMouseY;

extern TextRenderer flightWindowFont;

#endif //MISSION_CONTROL_FLIGHT_WINDOW_H_