#ifndef MISSION_CONTROL_VARIABLES_H_
#define MISSION_CONTROL_VARIABLES_H_

#include "Terrain.h"

extern unsigned int DISPLAY_WIDTH;
extern unsigned int DISPLAY_HEIGHT;

extern std::vector<TerrainSuperchunk> g_terrain;
extern Glider g_Glider;
extern std::vector<std::vector<int>> g_speeds2d;
extern std::vector<ADSBaircraft> g_ADSBaircraftList;

extern float g_FPS;

#endif // !MISSION_CONTROL_VARIABLES_H_