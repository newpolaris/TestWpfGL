#include "GraphicsEngine.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <memory>

#include "TriangleRenderer.h"

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

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

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

GLuint m_fbo = 0;
GLuint m_colorBuffer = 0, m_depthBuffer = 0;
int m_width = 0, m_height = 0;

void GraphicsEngine::renderToBuffer(char* imageBuffer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, m_width, m_height);

	m_triangle->draw();

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, m_width, m_height, GL_BGRA, GL_UNSIGNED_BYTE, imageBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Wait until resize is finished
	// glfwPollEvents();
}

void GraphicsEngine::resize(int width, int height)
{
	if (m_width == width && m_height == height)
		return;

	glDeleteFramebuffers(1, &m_fbo);
	glDeleteRenderbuffers(1, &m_colorBuffer);
	glDeleteRenderbuffers(1, &m_depthBuffer);
	m_fbo = 0;
	m_colorBuffer = 0;
	m_depthBuffer = 0;

	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenRenderbuffers(1, &m_colorBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_colorBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);

	glGenRenderbuffers(1, &m_depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_width = width;
	m_height = height;
}

