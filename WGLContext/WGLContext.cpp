#include "WGLContext.h"

#include <glad/glad.h>
#include <GL/wgl.h>
#include <GL/wglext.h>

#include <string>
#include <sstream>
#include <vector>

#pragma comment(lib, "opengl32.lib")

HGLRC GLContextCreate(HDC hDC) {
	if (hDC == NULL)
		return NULL;

	UINT PixelFormat = 0;
	PIXELFORMATDESCRIPTOR oPixelFormatDes = { 0 };
	oPixelFormatDes.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	oPixelFormatDes.nVersion = 1;
	oPixelFormatDes.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	oPixelFormatDes.iPixelType = PFD_TYPE_RGBA;
	oPixelFormatDes.cColorBits = 16;
	oPixelFormatDes.cDepthBits = 24;
	oPixelFormatDes.cStencilBits = 8;
	oPixelFormatDes.iLayerType = PFD_MAIN_PLANE;

	PixelFormat = ChoosePixelFormat(hDC, &oPixelFormatDes);

	if (!SetPixelFormat(hDC, PixelFormat, &oPixelFormatDes))
		return NULL;

	HGLRC hFakeGLRC = wglCreateContext(hDC);
	if (!hFakeGLRC)
		return NULL;

	if (!wglMakeCurrent(hDC, hFakeGLRC))
		return NULL;

	// TODO: from GLFW 
	// check if supported just like (glfwExtensionSupported("GL_ARB_debug_output"))

	int flags = 0;
#if _DEBUG
	flags = WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

	const int major = 4;
	const int minor = 3;
	const int profile = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
	static const int att[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor,
		WGL_CONTEXT_FLAGS_ARB, flags,
		WGL_CONTEXT_PROFILE_MASK_ARB, profile,
		0, 0
	};

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
		(PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

	HGLRC hglrc = wglCreateContextAttribsARB(hDC, NULL, att);
	if (!hglrc)
		return NULL;

	if (!wglMakeCurrent(NULL, NULL))
		return NULL;

	if (!wglDeleteContext(hFakeGLRC))
		return NULL;

	if (!wglMakeCurrent(hDC, hglrc)) {
		if (!wglDeleteContext(hglrc))
			return NULL;
		return NULL;
	}

	return hglrc;
}

static void APIENTRY glDebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	GLvoid* userParam)
{
	// ignore these non-significant error codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131184)
		return;

	std::wstringstream output;
	output << "---------- OPENGL CALLBACK -----------" << std::endl;
	output << "SOURCE: ";
	switch (source) {
	case GL_DEBUG_SOURCE_API:
		output << "WINDOW_SYSTEM";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		output << "SHADER_COMPILER";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		output << "THIRD_PARTY";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		output << "APPLICATION";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		output << "OTHER";
		break;
	}
	output << std::endl;

	output << "TYPE: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		output << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		output << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		output << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		output << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		output << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		output << "OTHER";
		break;
	}
	output << std::endl;

	output << "SEVERITY : ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		output << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		output << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		output << "HIGH";
		break;
	}
	output << std::endl;
	output << message << std::endl;
	OutputDebugString(output.str().c_str());
}

WGLContext::~WGLContext()
{
	destory();
}

bool WGLContext::create(HWND hWnd) {
	m_hWnd = hWnd;
	m_hDC = GetDC(hWnd);
	m_hRC = GLContextCreate(m_hDC);
	if (m_hRC == NULL)
		return false;

	gladLoadGL();

	// check list "http://glew.sourceforge.net/basic.html"
	// use arb instead https://github.com/openwebos/qt/blob/master/src/opengl/qglextensions.cpp
	// GLEW_ARB_vertex_program

#if _DEBUG
// Set debug callback
	if (glDebugMessageCallback != NULL) {
		// glDebugMessageCallback(glDebugCallback, NULL);
	}
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif // _DEBUG

	return true;
}

void WGLContext::destory()
{
}

void WGLContext::makeCurrent()
{
	wglMakeCurrent(m_hDC, m_hRC);

    RECT rc;
    GetClientRect(m_hWnd, &rc);

	glViewport(0, 0, rc.right - rc.left, rc.bottom - rc.top);
}

void WGLContext::present()
{
	SwapBuffers(m_hDC);
}


