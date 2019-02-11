#ifndef MISSION_CONTROL_WEATHER_DATA_H_
#define MISSION_CONTROL_WEATHER_DATA_H_

#include "stdHeaders.h"

#include <stdio.h>
#include <tchar.h>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>

class colours {
public:
	int violet[3] = { 128,0,128 };
	int blue_Violet[3] = { 160,32,192 };
	int amethyst[3] = { 128,96,192 };
	int aqua[3] = { 0,255,255 };
	int deep_Blue_Sky[3] = { 0,192,192 };
	int dodger_Blue[3] = { 30,144,255 };
	int dark_Cerulean[3] = { 16,78,139 };
	int green[3] = { 0,139,0 };
	int lime[3] = { 0,205,0 };
	int chartreuse[3] = { 127,255,0 };
	int chartreuse_Yellow[3] = { 238,238,0 };
	int gold[3] = { 254,215,0 };
	int mango_Tango[3] = { 205,133,0 };
	int Silver[3] = { 192,192,192 };
};

std::vector<std::vector<int>> get_Speed_Matrix(std::string file_Path, std::vector<std::vector<int>> &matrix, int level);
int getWindSpeedWithEastingNorthing(glm::vec3 pos);

#endif // !MISSION_CONTROL_WEATHER_DATA_H_
