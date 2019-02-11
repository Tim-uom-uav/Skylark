#include "stdHeaders.h"
#include "Terrain.h"

std::vector<TerrainSuperchunk> g_terrain;
float g_FPS;

Glider g_Glider;
std::vector<std::vector<int>> g_speeds2d(799, std::vector<int>(613));
std::vector<ADSBaircraft> g_ADSBaircraftList;

unsigned int DISPLAY_WIDTH;
unsigned int DISPLAY_HEIGHT;