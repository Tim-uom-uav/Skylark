#ifndef MISSION_CONTROL_FLIGHT_WINDOW_HUD_H_
#define MISSION_CONTROL_FLIGHT_WINDOW_HUD_H_

#include "stdHeaders.h"
#include "Class_Shader.h"
#include "Class_Text.h"

int drawFlightWindowHUD();
int setupFlightWindowHUD();

extern GLuint flightWinArtificialHorizonTex;
extern unsigned int flightWinArtificialHorizonVBO, flightWinArtificialHorizonVAO, flightWinArtificialHorizonEBO;
extern Shader flightWinArtificialHorizonShader;
extern TextRenderer flightWindowHUDfont;

extern GLchar* flightWindowHUDfragShader;
extern GLchar* flightWindowHUDvertShader;

#endif // !MISSION_CONTROL_FLIGHT_WINDOW_HUD_H_
