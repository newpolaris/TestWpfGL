#include "GraphicsEngine.h"

#include <Windows.h>
#include <memory>
#include <cassert>
#include <glad/glad.h>

#include "TriangleRenderer.h"
#include "WGLContext.h"

class TriangleRender;
class WGLContext;

static WGLContext* context = nullptr;

class GraphicsEngineImpl
{
public:

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	~GraphicsEngineImpl();

	bool create(int width, int height, bool bVisible);
	void destroy();
	void render();
	void renderToBuffer(char* imageBuffer);
	void resize(int width, int height);

	TriangleRender* m_triangle = nullptr;

	HWND m_hWindow = 0;
	GLuint m_fbo = 0;
	GLuint m_colorBuffer = 0, m_depthBuffer = 0;
	int m_width = 0, m_height = 0;
};


GraphicsEngineImpl::~GraphicsEngineImpl()
{
	destroy();
}

LRESULT CALLBACK GraphicsEngineImpl::WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

bool GraphicsEngineImpl::create(int width, int height, bool isVisible)
{
	// window requires at least size 1
	assert(width > 0 || height > 0);

	LPCTSTR lpszClass = TEXT("STATIC");

	WNDCLASS WndClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = 0;
	WndClass.hIcon = 0;
	WndClass.hCursor = 0;
	WndClass.hbrBackground = 0;
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	RegisterClass(&WndClass);

	// create minimal window
	m_hWindow = CreateWindow(lpszClass, TEXT("dummy"), WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP,
		0, 0, width, height,
		NULL, (HMENU)NULL, 0, NULL);

	if (m_hWindow == 0)
		return false;

	ShowWindow(m_hWindow, isVisible ? SW_SHOW : SW_HIDE);

	// HACK: prevent make multiple context
	//       함수 호출 과정에서 UI 이벤트 처리에 따라 새로 들어오면,
	//       context 가 변경될 가능성이 있다
	//       'wglDXRegisterObjectNV' 함수에서 resize 도중 render 가 들어와 문제였음
	if (context == nullptr) {
		context = new WGLContext();
		if (!context->create(m_hWindow))
			return false;
	}

	m_triangle = new TriangleRender();
	if (!m_triangle)
		return false;
	if (!m_triangle->create())
		return false;

	return true;
}

void GraphicsEngineImpl::destroy()
{
	if (m_triangle) {
		context->makeCurrent();
		m_triangle->destroy();
		delete m_triangle;
		m_triangle = nullptr;
	}

	if (m_hWindow) {
		DestroyWindow(m_hWindow);
		m_hWindow = 0;
	}
}

void GraphicsEngineImpl::render()
{
	context->makeCurrent();

	RECT rect;
	GetWindowRect(m_hWindow, &rect);
	GLsizei width = rect.right - rect.left;
	GLsizei height = rect.bottom - rect.top;
	glViewport(0, 0, width, height);

	m_triangle->draw();

	context->present();
}

void GraphicsEngineImpl::renderToBuffer(char* imageBuffer)
{
	context->makeCurrent();

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, m_width, m_height);

	m_triangle->draw();

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, m_width, m_height, GL_BGRA, GL_UNSIGNED_BYTE, imageBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 함수 작동 도중 다른 곳에서 context 를 바꾸면
void GraphicsEngineImpl::resize(int width, int height)
{
	if (m_width == width && m_height == height)
		return;

	context->makeCurrent();

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

GraphicsEngine::GraphicsEngine()
{
	pImpl = new GraphicsEngineImpl();
}

GraphicsEngine::~GraphicsEngine()
{
	destroy();
	delete pImpl;
	pImpl = 0;
}

bool GraphicsEngine::create(int width, int height, bool bVisible)
{
	assert(pImpl);
	return pImpl->create(width, height, bVisible);
}

void GraphicsEngine::destroy()
{
	if (pImpl) {
		pImpl->destroy();
	}
}

void GraphicsEngine::render()
{
	assert(pImpl);
	pImpl->render();
}

void GraphicsEngine::renderToBuffer(char* imageBuffer)
{
	assert(pImpl);
	pImpl->renderToBuffer(imageBuffer);
}

void GraphicsEngine::resize(int width, int height)
{
	assert(pImpl);
	pImpl->resize(width, height);
}
