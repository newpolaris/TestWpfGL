#include "GraphicsEngine.h"

#include <GL/glew.h>
#include <GL/wglew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <memory>

GraphicsEngine::~GraphicsEngine()
{
	destroy();
}

bool GraphicsEngine::create(int width, int height)
{
	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

	m_window = glfwCreateWindow(width, height, "Hidden OpenGL window", NULL, NULL);

	if (!m_window)
		return false;

	glfwMakeContextCurrent(m_window);
	glfwSwapInterval(1);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		return false;

	m_triangle = new TriangleRender();
	if (!m_triangle)
		return false;
	if (!m_triangle->create())
		return false;

	return true;
}

void GraphicsEngine::destroy()
{
	if (m_triangle) {
		delete m_triangle;
		m_triangle = nullptr;
	}
	if (m_window) {
		glfwDestroyWindow(m_window);
		glfwTerminate();

		m_window = nullptr;
	}
}

void GraphicsEngine::render()
{
	int width, height;
	glfwGetWindowSize(m_window, &width, &height);
	glViewport(0, 0, width, height);

	m_triangle->draw();

	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

void GraphicsEngine::resize(int width, int height)
{
	glfwSetWindowSize(m_window, width, height);
}

