#include "DxLib.h"

#include <Windows.h>
#include <TriangleRenderer.h>
#include <cassert>

#include "Direct3D9Driver.h"

struct DxGLRenderImpl
{
public:

	void resize(int width, int height);
	void render();

	void* getBackBuffer();

	bool create();
	void destroy();

	D3D9Driver* m_driver = NULL;
	TriangleRender* m_triangle = NULL;

	D3D9FrameBufferPtr m_fbo;
};

void DxGLRenderImpl::destroy()
{
	if (m_triangle) {
		m_triangle->destroy();
		delete m_triangle;
		m_triangle = nullptr;
	}

	m_fbo = nullptr;
}

void DxGLRenderImpl::resize(int width, int height)
{
	m_driver->makeCurrent();

	// 화면 surface 사용하지 않으므로 update 하지 않는다
	// m_driver->resize(width, height);

	// recreate framebuffer
	m_fbo = m_driver->createFrameBuffer(width, height);

	glViewport(0, 0, width, height);
}

bool DxGLRenderImpl::create()
{
	m_driver = getOrCreateDriver();
	if (!m_driver)
		return false;

	m_triangle = new TriangleRender();
	if (!m_triangle)
		return false;
	if (!m_triangle->create())
		return false;

	return true;
}

void DxGLRenderImpl::render()
{
	assert(m_fbo != nullptr);
	assert(m_triangle != nullptr);

	m_driver->makeCurrent();

	m_fbo->bind();
	m_triangle->draw();
	m_fbo->unbind();
}

void* DxGLRenderImpl::getBackBuffer()
{
	if (!m_fbo)
		return nullptr;
	return m_fbo->m_color;
}

DxGLRender::DxGLRender() :
	m_pImpl(new DxGLRenderImpl())
{
}

DxGLRender::~DxGLRender()
{
	delete m_pImpl;
	m_pImpl = nullptr;
}

bool DxGLRender::create()
{
	assert(m_pImpl);
	return m_pImpl->create();
}

void DxGLRender::destroy()
{
	assert(m_pImpl);
	m_pImpl->destroy();
}

void DxGLRender::resize(int width, int height)
{
	assert(m_pImpl);
	m_pImpl->resize(width, height);
}

void DxGLRender::render()
{
	assert(m_pImpl);
	m_pImpl->render();
}

void* DxGLRender::getBackBuffer() const
{
	assert(m_pImpl);
	return m_pImpl->getBackBuffer();
}

