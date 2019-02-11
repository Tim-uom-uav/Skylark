/**************************************************************************************************
*
*	Class_ADBSaircraft.cpp
*
*	Implementation of class that holds all the information gleaned from aircraft transponders off
*	the ADBS exchange. Also deals with updating their positions, predicting their current position,
*	and displaying the aircraft and relevant data on the 2D map view and the 3D flight window.
*
*	Changelog:
*	20.08.2018, Peter Naylor - Created
*	23.08.2018, Peter Naylor - Moved and slightly modified Flight Window rendering to map view
*
*	TODO
*	-> Work out wtf is wrong with text positioning matrices. Suspect at text rendering end.
*
***************************************************************************************************/
#include "stdHeaders.h"
#include "MapWindow.h"
#include "FlightWindow.h"
#include <chrono>

ADSBaircraft::ADSBaircraft() {

}
ADSBaircraft::~ADSBaircraft() {

}

ADSBaircraft::ADSBaircraft(std::string ICAO, std::string Reg, std::string Mdl, int WTC, int Species, bool Mil) {
	icao = ICAO;
	reg = Reg;
	mdl = Mdl;
	wtc = WTC;
	species = Species;
	mil = Mil;

	//Now set it up for drawing...
	// build and compile our shader zprogram
	glDeleteTextures(1, &texture);

	tileShader = Shader(vertShader, fragShader);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	float vertices[] = {
		// positions				// texture coords
		5000.0f,  5000.0f, 0.0f,   1.0f, 1.0f, // top right
		5000.0f,  0.0000f, 0.0f,   1.0f, 0.0f, // bottom right
		0.0000f,  0.0000f, 0.0f,   0.0f, 0.0f, // bottom left
		0.0000f,  5000.0f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// load and create a texture 
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int texWidth, texHeight;
	// Load textures
	std::string file = "data/Aircraft Tiles/Twin_Engine_Airliner.png";
	unsigned char* image = SOIL_load_image(file.c_str(), &texWidth, &texHeight, 0, SOIL_LOAD_RGB);
	if (!image) {
		std::cout << "FAILED TO LOAD IMAGE" << std::endl;
	}

	GLenum ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR) {
		std::cout << "SETUP GL_ERROR: " << ErrorCheckValue << std::endl;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	SOIL_free_image_data(image);
	glGenerateMipmap(GL_TEXTURE_2D);
	tileShader.use();
	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR) {
		std::cout << "GL_ERROR: " << ErrorCheckValue << std::endl;
	}
}

int ADSBaircraft::updatePos(glm::vec4 newPos, float newHeading, float newAirspeed, float newClimbRate, long long lastPing) {
	lastSeenPos = glm::vec4(util_latLongToOsGrid(glm::vec2(newPos.x, newPos.y)), newPos.z*0.3048, newPos.w*0.3048);
	lastSeenTime = lastPing;
	float airspeed_in_m_s = newAirspeed*0.51444f;
	vel = glm::vec3(airspeed_in_m_s*sin(newHeading), airspeed_in_m_s*cos(newHeading), newClimbRate*(0.3048/60)); //get everything in m/s
	return 1;
}

int ADSBaircraft::drawToFlightWindow(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	return 1;
}

int ADSBaircraft::drawToMapWindow(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	// bind Texture
	glBindTexture(GL_TEXTURE_2D, texture);
	// render container
	tileShader.use();
	tileShader.setMat4("view", viewMatrix);
	tileShader.setMat4("projection", projectionMatrix);
	glm::mat4 modelMatrix = glm::mat4(1.0);
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	glm::vec3 extrapolatedPos = glm::vec3(lastSeenPos.x + (vel.x*(ms.count() - lastSeenTime) / 1000), lastSeenPos.y + (vel.y*(ms.count() - lastSeenTime) / 1000), 0.5);
	modelMatrix = glm::translate(modelMatrix, extrapolatedPos);
	modelMatrix = glm::rotate(modelMatrix, float(atan2(vel.y, vel.x)-3.1416/2), glm::vec3(0,0,1)); //rotate picture to correct heading
	tileShader.setMat4("model", modelMatrix);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	GLenum ErrorCheckValue = glGetError();
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //Unbind

	glm::vec4 mapWindowTextPos = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	mapWindowTextPos = projectionMatrix*viewMatrix*modelMatrix*mapWindowTextPos;
	mapWindowFont.RenderText(icao, mapWindowTextPos.x, mapWindowTextPos.y, 1.0f, glm::vec3(0.2f, 0.0f, 0.0f));

	return 1;
}