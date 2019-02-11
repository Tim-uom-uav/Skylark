/**********************************************************************************************************************
*
*	FlightWindowHUD.cpp
*
*	Changelog
*	25.08.2018, Peter - Created, basic overhead etc got working
*	27.08.2018, Peter - Added in framebuffer for drawing artificial horizon in screen centre
*
************************************************************************************************************************/

#include "stdHeaders.h"
#include "FlightWindowHUD.h"
#include "FlightWindow.h"
#include "Weather_Data.h"

int genArtificialHorizon();
int drawArtificialHorizonPitchLines();

GLuint flightWinArtificialHorizonTex;
Shader flightWinArtificialHorizonShader;

int setupFlightWindowHUD() {
	// build and compile our shader zprogram
	flightWinArtificialHorizonShader = Shader(flightWindowHUDvertShader, flightWindowHUDfragShader);
	return 1;
}

int drawFlightWindowHUD() {
	glDisable(GL_DEPTH_TEST); //Make sure we draw over everything
	int windSpeed = getWindSpeedWithEastingNorthing(flightWindowCamera.getPosition());
	flightWindowFont.RenderText("Wind Speed: "+std::to_string(windSpeed)+"m/s", (flightWindowWidth/2)-50, 18, 1, glm::vec3(0.9, 1.0, 1.0));
	flightWindowFont.RenderText("Alt: " + std::to_string(int(flightWindowCamera.getPosition().z)) + "m", (flightWindowWidth / 2) - 40, 30, 1, glm::vec3(0.9, 1.0, 1.0));
	//genArtificialHorizon();
	glEnable(GL_DEPTH_TEST);
	return 1;
}

int drawArtificialHorizonPitchLines() {
	float vertices[] = {
		0.0f,  0.5f, // Vertex 1 (X, Y)
		0.5f, -0.5f, // Vertex 2 (X, Y)
		-0.5f, -0.5f  // Vertex 3 (X, Y)
	};
	GLuint vbo;
	glGenBuffers(1, &vbo); // Generate 1 buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	const GLchar* vertShader =
	{
		"#version 400\n"\
		"in vec2 position;\n"\

		"void main()\n"\
		"{\n"\
		"	gl_Position = vec4(position, 0.0, 1.0);\n"\
		"}\n"
	};
	const GLchar* fragShader =
	{
		"#version 400\n"\

		"out vec4 outColor;\n"\
		"void main()\n"\
		"{\n"\
			"outColor = vec4(0.6, 0.6, 0.6, 1.0);\n"\
		"}\n"
	};

	Shader shader(vertShader, fragShader);
	shader.use();

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	GLenum ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR) {
		std::cout << "LINES DRAW ERROR: " << ErrorCheckValue << std::endl;
	}

	//clear up after ourselves
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	return 1;
}

int genArtificialHorizon() {
	unsigned int flightWinArtificialHorizonVBO, flightWinArtificialHorizonVAO, flightWinArtificialHorizonEBO;
	//destroy any previous textures that may have been loaded...
	glDeleteTextures(1, &flightWinArtificialHorizonTex);
	glDeleteVertexArrays(1, &flightWinArtificialHorizonVAO);
	glDeleteBuffers(1, &flightWinArtificialHorizonVBO);
	glDeleteBuffers(1, &flightWinArtificialHorizonEBO);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	float vertices[] = {
		// positions		   // texture coords
		0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
		0.5f,  -.5f, 0.0f,   1.0f, 0.0f, // bottom right
		-.5f,  -.5f, 0.0f,   0.0f, 0.0f, // bottom left
		-.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	glGenVertexArrays(1, &flightWinArtificialHorizonVAO);
	glGenBuffers(1, &flightWinArtificialHorizonVBO);
	glGenBuffers(1, &flightWinArtificialHorizonEBO);

	glBindVertexArray(flightWinArtificialHorizonVAO);

	glBindBuffer(GL_ARRAY_BUFFER, flightWinArtificialHorizonVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, flightWinArtificialHorizonEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);


	// The stuff we're going to render to
	GLuint frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glGenTextures(1, &flightWinArtificialHorizonTex);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, flightWinArtificialHorizonTex);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 500, 500, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, flightWinArtificialHorizonTex, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	//Did it work? Is our baby framebuffer ok?
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "HUD GENERATION ERROR: FRAMEBUFFER NOT OK (Also potential memory leak =] )" << std::endl;
		return 0;
	}

	// Render to our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	flightWinArtificialHorizonShader.use();
	glm::mat4 translationMatrix = glm::mat4(1.0f);
	flightWinArtificialHorizonShader.setMat4("model", translationMatrix);
	flightWinArtificialHorizonShader.setMat4("view", glm::mat4(1.0f));
	flightWinArtificialHorizonShader.setMat4("projection", glm::mat4(1.0f));

	glClearColor(1.0f, 1.0f, 0.5f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT);

	/********************************************
	*	Draw to Framebuffer here!
	*********************************************/
	drawArtificialHorizonPitchLines();
	//flightWindowHUDfont.RenderText("TEST_HUD_HORIZON_TEXT...", -20, -5, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	/********************************************
	*	End drawing to framebuffer!
	*********************************************/

	glDeleteFramebuffers(1, &frameBuffer); //delete!
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //Unbind

	// bind Texture
	glBindTexture(GL_TEXTURE_2D, flightWinArtificialHorizonTex);
	// render container
	flightWinArtificialHorizonShader.use();
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(flightWinArtificialHorizonVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//Clean up after ourselves
	glDeleteTextures(1, &flightWinArtificialHorizonTex);
	glDeleteVertexArrays(1, &flightWinArtificialHorizonVAO);
	glDeleteBuffers(1, &flightWinArtificialHorizonVBO);
	glDeleteBuffers(1, &flightWinArtificialHorizonEBO);
	return 1;
}

GLchar* flightWindowHUDfragShader =
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
/*"	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(texture1, TexCoord).r);\n"\
"	FragColor = vec4(texture(texture1, TexCoord)) * sampled;\n"\*/
GLchar* flightWindowHUDvertShader =
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