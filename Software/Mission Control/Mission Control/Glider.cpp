#include "stdHeaders.h"

Glider::Glider() {
	pos = { 0,0,0,0 };
	velocity = { 0,0,0,0 };
	rotation = { 0,0,0,0 };
	angularVelocity = { 0,0,0,0 };
}

glm::vec4 Glider::getPos() {
	return pos;
}

int Glider::setupMapIcon() {
	glDeleteTextures(1, &texture);

	tileShader = Shader(vertShader, fragShader);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	float vertices[] = {
		// positions				// texture coords
		2000.00f,  4000.00f, 0.0f,   1.0f, 1.0f, // top right
		2000.00f,  -4000.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-2000.0f,  -4000.0f, 0.0f,   0.0f, 0.0f, // bottom left
		-2000.0f,  4000.00f, 0.0f,   0.0f, 1.0f  // top left 
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
	std::string file = "data/Aircraft Tiles/Balloon_Glider.png";
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
	return 1;
}
int Glider::drawMapIcon(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	// bind Texture
	glBindTexture(GL_TEXTURE_2D, texture);
	// render container
	tileShader.setMat4("view", viewMatrix);
	tileShader.setMat4("projection", projectionMatrix);
	tileShader.use();
	tileShader.setMat4("view", viewMatrix);
	tileShader.setMat4("projection", projectionMatrix);
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(pos.x,pos.y,0.5f));
	modelMatrix = glm::rotate(modelMatrix, -3.141593f/2.0f+atan2(flightWindowViewDirection.y, flightWindowViewDirection.x), glm::vec3(0.0f, 0.0f, 1.0f));
	tileShader.setMat4("model", modelMatrix);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	GLenum ErrorCheckValue = glGetError();
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //Unbind
	return 1;
}

glm::vec4 Glider::setPos(glm::vec4 newPos) {
	glm::vec4 lastPos = pos;
	pos = newPos;
	positionLog.emplace_back(pos);
	return lastPos;
}

std::vector<glm::vec4> Glider::getPosLog() {
	return positionLog;
}

glm::vec3 Glider::setFlightWindowViewDirection(glm::vec3 newDir) {
	glm::vec3 oldDir = flightWindowViewDirection;
	flightWindowViewDirection = newDir;
	return oldDir;
}
glm::vec3 Glider::getFlightWindowViewDirection() {
	return flightWindowViewDirection;
}