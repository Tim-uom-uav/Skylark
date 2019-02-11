#ifndef MISSION_CONTROL_CLASS_GLIDER_H_
#define MISSION_CONTROL_CLASS_GLIDER_H_

#include "stdHeaders.h"
#include "Class_Shader.h"

class Glider {
private:
	glm::vec4 pos; //Lat, long, alt, time updated
	glm::vec4 velocity; //x, y, z, time updated
	std::vector<glm::vec4> positionLog; //List of previous positions and times
	glm::vec4 rotation; //Quaternion - x, y, z, w
	glm::vec4 angularVelocity; //Euler - x, y, z, time updated
	//For drawing on the map view...
	Shader tileShader;
	unsigned int texture;
	unsigned int VBO, VAO, EBO;
	glm::mat4 translationMatrix;
	glm::vec3 flightWindowViewDirection;
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
	Glider();
	int setupMapIcon();
	int drawMapIcon(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
	glm::vec4 getPos();
	glm::vec4 setPos(glm::vec4 newPos);
	glm::vec3 setFlightWindowViewDirection(glm::vec3 newDir);
	glm::vec3 getFlightWindowViewDirection();
	std::vector<glm::vec4> getPosLog();
};

#endif // !MISSION_CONTROL_CLASS_GLIDER_H_
