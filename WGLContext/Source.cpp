#include <string>
#include <sstream>
#include <vector>

#include <Windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#pragma comment(lib, "opengl32.lib")

#define EXPORT extern "C" __declspec(dllexport)

EXPORT int GLCreate(const void* handle);
EXPORT int Render();

class WGLContext
{
public:

	~WGLContext();

	bool create(HWND hWnd);
	void destory();
	void makeCurrent();
	void present();

	HDC m_hDC = 0;
	HWND m_hWnd = 0;
	HGLRC m_hRC = 0;
};

class TriangleRender
{
public:

	~TriangleRender();

	bool create();
	bool createGeometry();
	bool createProgram();
	void draw();

	GLuint createShader(GLenum type, const char* shaderCode);

	GLuint m_vertexShader;
	GLuint m_fragmentShader;
	GLuint m_program;
	GLuint m_vao;
	GLuint m_vbo;
};

namespace {
	WGLContext* context = nullptr;
	TriangleRender* render = nullptr;
}

extern "C" {

	int GLCreate(const void* handle)
	{
		context = new WGLContext();
		if (!context->create((HWND)handle))
			return -1;

		char* szVersion = (char*)glGetString(GL_VERSION);
		GLint last_viewport[4];
		glGetIntegerv(GL_VIEWPORT, last_viewport);
		GLint last_scissor_box[4];
		glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glGetIntegerv(GL_VIEWPORT, last_viewport);
		glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);

		render = new TriangleRender();
		if (!render->create())
			return -1;

		return 0;
	}

	int Render()
	{
		if (context) {
			context->makeCurrent();
			if (render)
				render->draw();
			context->present();
		}
		return 0;
	}
}

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
	const void* userParam)
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

	// Test C++ 2017 syntax
	if (auto error (glewInit()); error != GLEW_OK)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		return false;
	}

	// check list "http://glew.sourceforge.net/basic.html"
	// use arb instead https://github.com/openwebos/qt/blob/master/src/opengl/qglextensions.cpp
	// GLEW_ARB_vertex_program

#if _DEBUG
// Set debug callback
	if (glDebugMessageCallback != NULL) {
		glDebugMessageCallback(glDebugCallback, NULL);
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

