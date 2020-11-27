#pragma once

#include <glad/glad.h>

class TriangleRender
{
public:

	~TriangleRender();

	bool create();
	bool createGeometry();
	bool createProgram();
	void destroy();

	void draw();

	GLuint createShader(GLenum type, const char* shaderCode);

	GLuint m_vertexShader;
	GLuint m_fragmentShader;
	GLuint m_program;
	GLuint m_vao;
	GLuint m_vbo;
};


