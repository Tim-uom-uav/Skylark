/**************************************************************************************************
*
*	Weather Data Reader.cpp
*
*	Changelog
*	Yahya - created
*	16.04.2018, Peter - chopped it into a header file and a cpp file, plus some minor tweaks
*	24.08.2018, Peter - added a function to (roughly) allow OS eastings/northings to access data
*
***************************************************************************************************/

#include "Weather_Data.h"
#include "Variables.h"

int getWindSpeedWithEastingNorthing(glm::vec3 pos) {
	glm::vec2 windSpeedIndexes = glm::vec2(pos.x, pos.y);
	windSpeedIndexes /= 10000;
	windSpeedIndexes.x -= 300;
	windSpeedIndexes.y -= 200;
	if (windSpeedIndexes.x < 0) {
		windSpeedIndexes.x = 0;
	}
	if (windSpeedIndexes.y < 0) {
		windSpeedIndexes.y = 0;
	}
	return int(g_speeds2d[windSpeedIndexes.x][windSpeedIndexes.y] * 0.514);
}

int get_Speed(int r, int g, int b) {
	colours colours;
	if (colours.violet[0] == r && colours.violet[1] == g && colours.violet[2] == b) {
		return 10;
	}
	else if (colours.blue_Violet[0] == r && colours.blue_Violet[1] == g && colours.blue_Violet[2] == b) {
		return 20;
	}
	else if (colours.amethyst[0] == r && colours.amethyst[1] == g && colours.amethyst[2] == b) {
		return 30;
	}
	else if (colours.aqua[0] == r && colours.aqua[1] == g && colours.aqua[2] == b) {
		return 40;
	}
	else if (colours.deep_Blue_Sky[0] == r && colours.deep_Blue_Sky[1] == g && colours.deep_Blue_Sky[2] == b) {
		return 50;
	}
	else if (colours.dodger_Blue[0] == r && colours.dodger_Blue[1] == g && colours.dodger_Blue[2] == b) {
		return 60;
	}
	else if (colours.dark_Cerulean[0] == r && colours.dark_Cerulean[1] == g && colours.dark_Cerulean[2] == b) {
		return 70;
	}
	else if (colours.green[0] == r && colours.green[1] == g && colours.green[2] == b) {
		return 80;
	}
	else if (colours.lime[0] == r && colours.lime[1] == g && colours.lime[2] == b) {
		return 90;
	}
	else if (colours.chartreuse[0] == r && colours.chartreuse[1] == g && colours.chartreuse[2] == b) {
		return 100;
	}
	else if (colours.chartreuse_Yellow[0] == r && colours.chartreuse_Yellow[1] == g && colours.chartreuse_Yellow[2] == b) {
		return 110;
	}
	else if (colours.Silver[0] == r && colours.Silver[1] && colours.Silver[2]) {
		//std::cout << "Found Land!" << std::endl;
		return -1;
	}
	else {
		std::cout << "Colour " << r << "," << g << "," << b << " not recognised";
		return -2;
	};
}

int get_Speed_Surface(int r, int g, int b) {
	colours colours;
	if (colours.violet[0] == r && colours.violet[1] == g && colours.violet[2] == b) {
		return 3;
	}
	else if (colours.blue_Violet[0] == r && colours.blue_Violet[1] == g && colours.blue_Violet[2] == b) {
		return 6;
	}
	else if (colours.amethyst[0] == r && colours.amethyst[1] == g && colours.amethyst[2] == b) {
		return 9;
	}
	else if (colours.aqua[0] == r && colours.aqua[1] == g && colours.aqua[2] == b) {
		return 12;
	}
	else if (colours.deep_Blue_Sky[0] == r && colours.deep_Blue_Sky[1] == g && colours.deep_Blue_Sky[2] == b) {
		return 15;
	}
	else if (colours.dodger_Blue[0] == r && colours.dodger_Blue[1] == g && colours.dodger_Blue[2] == b) {
		return 20;
	}
	else if (colours.dark_Cerulean[0] == r && colours.dark_Cerulean[1] == g && colours.dark_Cerulean[2] == b) {
		return 25;
	}
	else if (colours.green[0] == r && colours.green[1] == g && colours.green[2] == b) {
		return 30;
	}
	else if (colours.lime[0] == r && colours.lime[1] == g && colours.lime[2] == b) {
		return 35;
	}
	else if (colours.chartreuse[0] == r && colours.chartreuse[1] == g && colours.chartreuse[2] == b) {
		return 40;
	}
	else if (colours.chartreuse_Yellow[0] == r && colours.chartreuse_Yellow[1] == g && colours.chartreuse_Yellow[2] == b) {
		return 45;
	}
	else if (colours.gold[0] == r && colours.gold[1] == g && colours.gold[2] == b) {
		return 50;
	}
	else if (colours.mango_Tango[0] == r && colours.mango_Tango[1] == g && colours.mango_Tango[2] == b) {
		return 55;
	}
	else {
		std::cout << "ERROR: Colour not found";
		return -1;
	};
}

std::vector<std::vector<int>> get_Speed_Matrix(std::string file_Path, std::vector<std::vector<int>> &matrix, int level) {
	//Create and load image variable
	boost::gil::rgb8_image_t img;
	boost::gil::png_read_image(file_Path, img);
	std::cout << "Reading wind speed image...";
	//Load speed values onto speeds2d
	for (int yPosition = 44; yPosition <= 656; yPosition++) {
		for (int xPosition = 0; xPosition <= 798; xPosition++) {

			//Point at specific pixel
			boost::gil::rgb8_pixel_t px = *const_view(img).at(xPosition, yPosition);

			//See if level we are looking at is at surface, since colour coding is slightly different
			if (level == 0) {
				matrix[xPosition][yPosition - 44] = get_Speed_Surface((int)px[0], (int)px[1], (int)px[2]);
			}
			else {
				matrix[xPosition][yPosition - 44] = get_Speed((int)px[0], (int)px[1], (int)px[2]);
			}

		}
	}
	std::cout << "done!" << std::endl;
	return matrix;
}