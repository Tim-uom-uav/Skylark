/**************************************************************************************************
*
*	Class_ADBSaircraft.h
*
*	Class that holds all the information gleaned from aircraft transponders off the ADBS exchange.
*	Also deals with updating their positions, predicting their current position, and displaying
*	the aircraft and relevant data on the 2D map view and the 3D flight window.
*
*	Changelog:
*	20.08.2018, Peter Naylor - Created
*	22.08.2018, Peter Naylor - Added extractFromJSON function and ADSBaircraft prototypes
*
***************************************************************************************************/

#ifndef MISSION_CONTROL_CLASS_ADSBAIRCRAFT_H_
#define MISSION_CONTROL_CLASS_ADSBAIRCRAFT_H_

#include "stdHeaders.h"
#include "Class_Shader.h"
#include <deque>
#include <iterator>

class ADSBaircraft {
private:
	std::string icao; // The six-digit hex code used to identify an aircraft
	std::string reg; // Aircraft registration, as pulled off ICAO database
	std::string mdl; // Aircraft model. ADSBexchange helpfully pulls this off an ICAO database. So useful, but not 100% reliable.
	glm::vec4 lastSeenPos; // Last reported Easting, Northing, GAlt Altitude (roughly above SL), Alt (alt in ft at standard pressure)
	long long lastSeenTime; //Last time we got a transponder ping in UNIX time
	int wtc; // Wake turbulence category 0-3, or "how much should we shit ourselves if we're gonna fly through its trail"
	int species; //0-8. What is it? For us, that controls what icon gets displayed
	glm::vec3 vel; //m/s in all three axes
	std::vector<glm::vec4> trail; //trail of previous positions and UNIX time - downloaded from server, NOT logged from repeated requests
	bool mil; // Will it shoot us down?
	bool gnd; // Or is it grounded?
	std::string from; // Where (might) it be coming from?
	std::string to; // Where (might) it be going?
	GLuint tileTexture;
	GLfloat *vertices;
	// Create Vertex Array Object
	GLuint tileVAO;
	GLuint tileVBO;
	GLuint tileEBO;
	Shader tileShader;
	unsigned int texture;
	int texState = -1; // Keeps track of tile texture loading. 0 is unloaded (blank tile), 1 is loaded, -1 is nothing
	//int textureLoadUnloadRequest = 0; // -1 for unload, +1 for load, 0 for nothing. Needed for multithread/asynchronous execution tracking
	unsigned int VBO, VAO, EBO;
	glm::mat4 translationMatrix;
	const GLchar* fragShader =
	{
		"#version 330 core\n"\
		"out vec4 FragColor;\n"\

		"in vec3 ourColor;\n"\
		"in vec2 TexCoord;\n"\

		// texture sampler
		"uniform sampler2D texture1;\n"\

		"void main()\n"\
		"{\n"\
		"FragColor = texture(texture1, TexCoord);\n"\
		"}\n"
	};
	const GLchar* vertShader =
	{
		"#version 330 core\n"\
		"layout(location = 0) in vec3 aPos;\n"\
		"layout(location = 1) in vec3 aColor;\n"\
		"layout(location = 2) in vec2 aTexCoord;\n"\
		"uniform mat4 model;\n"\
		"uniform mat4 view;\n"\
		"uniform mat4 projection;\n"\

		"out vec3 ourColor;\n"\
		"out vec2 TexCoord;\n"\

		"void main()\n"\
		"{\n"\
		"gl_Position = projection * view * model * vec4(aPos, 1.0);\n"\
		"ourColor = aColor;\n"\
		"TexCoord = vec2(aTexCoord.x, 1-aTexCoord.y);\n"\
		"}\n"
	};
public:
	ADSBaircraft();
	~ADSBaircraft();
	ADSBaircraft(std::string ICAO, std::string Reg, std::string Mdl, int WTC, int Species, bool Mil);
	int updatePos(glm::vec4 newPos, float newHeading, float newAirspeed, float newClimbRate, long long lastPing);
	int drawToMapWindow(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
	int drawToFlightWindow(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
};

inline std::vector<ADSBaircraft> extractAircraftFromJSON(std::string json) {
	std::vector<ADSBaircraft> aircraft;

	std::deque<std::string> splitString = util_string_split(json, '\n');
	for (unsigned int i = 0; i < splitString.size(); i++) {
		if (splitString[i].length() < 2 && i > splitString.size() - 5) {
			std::cout << "END OF HEADER" << std::endl;
			std::deque<std::string> newJson;
			for (unsigned int j = i+1; j < splitString.size(); j++) {
				newJson.emplace_back(splitString[j]); //add the extracted data fields to an Array!
				///Need to check what garbage characters get left on the end of the final text block
			}
			splitString = newJson;
			break;
		}
	}
	if (splitString.size() > 0) {
		std::deque<std::string> jsonArray = util_string_split(splitString[0], '{');
		for (unsigned int i = 0; i < jsonArray.size(); i++) {
			std::string currentString = jsonArray[i];
			if (currentString.find("\"Icao\"") != std::string::npos) { //Try to avoid the lack of ICAO error later if possible
				std::deque<std::string> aircraftData = util_string_split(currentString, ',');
				std::string result;
				result = util_get_JSON_Field(aircraftData, "\"Icao\"");
				std::string ICAO;
				if (result != "") { //An aircraft MUST have an ICAO beacon to be valid.
					ICAO = result;

					result = util_get_JSON_Field(aircraftData, "\"Reg\"");
					std::string Reg;
					if (result != "") {
						Reg = result;
					}
					else {
						Reg = "Unknown";
					}
					result = util_get_JSON_Field(aircraftData, "\"Mdl\"");
					std::string Mdl;
					if (result != "") {
						Mdl = result;
					}
					else {
						Mdl = "Unknown";
					}
					result = util_get_JSON_Field(aircraftData, "\"WTC\"");
					int WTC;
					if (result != "") {
						WTC = atoi(result.c_str());
					}
					else {
						WTC = -1;
					}
					result = util_get_JSON_Field(aircraftData, "\"Species\"");
					int Species;
					if (result != "") {
						Species = atoi(result.c_str());
					}
					else {
						Species = -1;
					}
					result = util_get_JSON_Field(aircraftData, "\"Mil\"").c_str();
					bool Mil;
					if (result == "true") {
						Mil = true;
					}
					else {
						Mil = false;
					}
					ADSBaircraft newAircraft(ICAO, Reg, Mdl, WTC, Species, Mil);
					glm::vec4 newPos;
					float lat;
					float lng;
					float alt;
					float gAlt;
					result = util_get_JSON_Field(aircraftData, "\"Lat\"");
					if (result != "") {
						lat = (float)atof(result.c_str());
					}
					else {
						lat = -1;
					}
					result = util_get_JSON_Field(aircraftData, "\"Long\"");
					if (result != "") {
						lng = (float)atof(result.c_str());
					}
					else {
						lng = -1;
					}
					result = util_get_JSON_Field(aircraftData, "\"Alt\"");
					if (result != "") {
						alt = (float)atof(result.c_str());
					}
					else {
						alt = -1;
					}
					result = util_get_JSON_Field(aircraftData, "\"GAlt\"");
					if (result != "") {
						gAlt = (float)atof(result.c_str());
					}
					else {
						gAlt = -1;
					}
					newPos = glm::vec4(lat, lng, alt, gAlt);
					float newHeading;
					result = util_get_JSON_Field(aircraftData, "\"Trak\"");
					if (result != "") {
						newHeading = (float)atof(result.c_str());
					}
					else {
						newHeading = -1;
					}
					float newAirspeed;
					result = util_get_JSON_Field(aircraftData, "\"Spd\"");
					if (result != "") {
						newAirspeed = (float)atof(result.c_str());
					}
					else {
						newAirspeed = -1;
					}
					float newClimbRate;
					result = util_get_JSON_Field(aircraftData, "\"Vsi\"");
					if (result != "") {
						newClimbRate = (float)atof(result.c_str());
					}
					else {
						newClimbRate = -1;
					}
					long long lastSeen;
					result = util_get_JSON_Field(aircraftData, "\"PosTime\"");
					if (result != "") {
						lastSeen = atoll(result.c_str());
					}
					else {
						lastSeen = -1;
					}
					newAircraft.updatePos(newPos, newHeading, newAirspeed, newClimbRate, lastSeen);
					aircraft.emplace_back(newAircraft);
				}
			}
		}
	}
	return aircraft;
}

#endif //MISSION_CONTROL_CLASS_ADSBAIRCRAFT_H_
