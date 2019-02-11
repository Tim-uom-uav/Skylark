/**************************************************************************************************
*	Setup.cpp
***************************************************************************************************/

#include "stdHeaders.h"
#include "Variables.h"
#include "Class_Text.h"
#include "Weather_Data.h"
#include "HTTPS_Get.h"
#include "Class_ADSBaircraft.h"

int importMapVertexData() {
	glfwMakeContextCurrent(flightWindow);
	g_Glider.setPos(glm::vec4(50.72, -3.53, 5000, 0));
	get_Speed_Matrix("Data/Forecasts/Wind Velocity/png/2018081212.f000.1000.skntnonenonenonenone.eur.gfs003.png", g_speeds2d, 2); //235, 354; 313,222

	//Setup the Terrain list
	for (int e = 0; e <= 600; e += 100) {
		for (int n = 0; n <= 1200; n += 100) {
			std::string squareName = util_EastingNorthingToGridLetter(glm::vec2(e * 1000, n * 1000), false);
			TerrainSuperchunk newChunk;
			newChunk.initSuperchunk(squareName, glm::vec2(e * 1000, n * 1000));
			g_terrain.emplace_back(newChunk);
		}
	}

	return 1;
}


