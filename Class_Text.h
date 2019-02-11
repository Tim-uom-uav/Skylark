/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#ifndef MISSION_CONTROL_CLASS_TEXT_H_
#define MISSION_CONTROL_CLASS_TEXT_H_

#include "stdHeaders.h"
#include "Class_Shader.h"
#include <map>

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
	GLuint Advance;     // Horizontal offset to advance to next glyph
};


// A renderer class for rendering text displayed by a font loaded using the 
// FreeType library. A single font is loaded, processed into a list of Character
// items for later rendering.
class TextRenderer
{
public:
	// Holds a list of pre-compiled Characters
	std::map<GLchar, Character> Characters;
	// Shader used for text rendering
	Shader TextShader;
	// Constructor
	TextRenderer();
	TextRenderer(GLuint width, GLuint height);
	int resize(GLuint width, GLuint height);
	// Pre-compiles a list of characters from the given font
	void Load(std::string font, GLuint fontSize);
	// Renders a string of text using the precompiled list of characters
	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color = glm::vec3(1.0f));
private:
	// Render state
	GLuint VAO, VBO;
	const GLchar* vertexShader = 
	{
		"#version 330 core\n"\
		"layout(location = 0) in vec4 vertex; \n"\
		"out vec2 TexCoords; \n"\

		"uniform mat4 projection; \n"\

		"void main()\n"\
		"{\n"\
		"	gl_Position = projection * vec4(vertex.xy, 0.0, 1.0); \n"\
		//projection *
		"	TexCoords = vertex.zw;\n"\
		"}\n"
	};
	const GLchar* fragShader =
	{
		"#version 330 core\n"\
		"in vec2 TexCoords;\n"\
		"out vec4 color;\n"\

		"uniform sampler2D text;\n"\
		"uniform vec3 textColor;\n"\

		"void main()\n"\
		"{\n"\
		"	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"\
		"	color = vec4(textColor, 1.0) * sampled;\n"\
		"}\n"
	};
};

#endif 