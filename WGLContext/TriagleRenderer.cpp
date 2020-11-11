#include "TriangleRenderer.h"

#include <stdint.h>
#include <vector>
#include <cmath>

const char* vertexShader = R"__(
#version 330 core

layout(location = 0) in vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0, 1);
}
)__";

const char* fragmentShader = R"__(
#version 330 core

out vec4 color_out;

void main()
{
    color_out = vec4(1.0, 0.5, 0.5, 1.0);
}
)__";

float vertices[] =
{
    0.0f, 1.0f,
    -1.0f, -1.0f,
    1.0f, -1.0f
};

TriangleRender::~TriangleRender()
{
}

bool TriangleRender::create()
{
	if (!createGeometry())
		return false;
	if (!createProgram())
		return false;
	return true;
}

bool TriangleRender::createGeometry()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

	return true;
}

bool TriangleRender::createProgram()
{
	m_vertexShader = createShader(GL_VERTEX_SHADER, vertexShader);
	m_fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShader);

	m_program = glCreateProgram();
    glAttachShader(m_program, m_vertexShader);
    glAttachShader(m_program, m_fragmentShader);
    glLinkProgram(m_program);

    GLint status = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE || m_program == 0)
    {
        const uint32_t kBufferSize = 512u;
        char log[kBufferSize];
        glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);

		return false;
    }
    return true;
}

void TriangleRender::draw()
{
	static float c = 0.f;
	c += 0.05f;
	glClearColor(sin(c), 0, cos(c), 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_program);
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

// expect null terminated shader source
GLuint TriangleRender::createShader(GLenum type, const char* shaderCode)
{
    GLuint id = glCreateShader(GL_FRAGMENT_SHADER);
	if (id == 0)
		return 0;

	glShaderSource(id, 1, &shaderCode, nullptr);
	glCompileShader(id);

    GLint compiled = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE)
    {
        GLint length = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::vector<GLchar> buffer(length + 1);
        glGetShaderInfoLog(id, length, 0, buffer.data());
        glDeleteShader(id);
        return 0;
    }
	return id;
}

