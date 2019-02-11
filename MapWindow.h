/**********************************************************************************************************************
*
*	MapWindow.h
*
***********************************************************************************************************************/

#ifndef MISSION_CONTROL_MAP_WINDOW_H_
#define MISSION_CONTROL_MAP_WINDOW_H_

#include "Class_Camera.h"
#include "Class_Text.h"
#include "Textured_map.h"

extern GLFWwindow* mapWindow;
int loadMapWindowData();

extern double mapWindowMouseX;
extern double mapWindowLastMouseX;
extern double mapWindowMouseY;
extern double mapWindowLastMouseY;
extern double mapWindowMouseDown;

extern float mapWindowWidth;
extern float mapWindowHeight;
extern OrthoCamera mapCamera;
extern TiledMap OSExplorerMap;

extern TextRenderer mapWindowFont;

#endif // !MISSION_CONTROL_MAP_WINDOW_H_
