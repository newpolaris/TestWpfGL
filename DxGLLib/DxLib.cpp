#include "DxLib.h"

#include <Windows.h>
#include <d3d9.h>

#include <glad/glad.h>
#include <glad/glad_wgl.h>
#include <cassert>
#include <memory>

#include <WGLContext.h>
#include <TriangleRenderer.h>

// CComPtr
#include <atlcomcli.h>
#pragma comment(lib, "oleaut32.lib")

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "opengl32.lib")

const char* GetDX9HResult(HRESULT hr)
{
	switch (hr) {
	case D3DERR_WRONGTEXTUREFORMAT:
		return "WRONGTEXTUREFORMAT";
	case D3DERR_UNSUPPORTEDCOLOROPERATION:
		return "UNSUPPORTEDCOLOROPERATION";
	case D3DERR_UNSUPPORTEDCOLORARG:
		return "UNSUPPORTEDCOLORARG";
	case D3DERR_UNSUPPORTEDALPHAOPERATION:
		return "UNSUPPORTEDALPHAOPERATION";
	case D3DERR_UNSUPPORTEDALPHAARG:
		return "UNSUPPORTEDALPHAARG";
	case D3DERR_TOOMANYOPERATIONS:
		return "TOOMANYOPERATIONS";
	case D3DERR_CONFLICTINGTEXTUREFILTER:
		return "CONFLICTINGTEXTUREFILTER";
	case D3DERR_UNSUPPORTEDFACTORVALUE:
		return "UNSUPPORTEDFACTORVALUE";
	case D3DERR_CONFLICTINGRENDERSTATE:
		return "CONFLICTINGRENDERSTATE";
	case D3DERR_UNSUPPORTEDTEXTUREFILTER:
		return "UNSUPPORTEDTEXTUREFILTER";
	case D3DERR_CONFLICTINGTEXTUREPALETTE:
		return "CONFLICTINGTEXTUREPALETTE";
	case D3DERR_DRIVERINTERNALERROR:
		return "DRIVERINTERNALERROR";

	case D3DERR_NOTFOUND:
		return "NOTFOUND";
	case D3DERR_MOREDATA:
		return "MOREDATA";
	case D3DERR_DEVICELOST:
		return "DEVICELOST";
	case D3DERR_DEVICENOTRESET:
		return "DEVICENOTRESET";
	case D3DERR_NOTAVAILABLE:
		return "NOTAVAILABLE";
	case D3DERR_OUTOFVIDEOMEMORY:
		return "OUTOFVIDEOMEMORY";
	case D3DERR_INVALIDDEVICE:
		return "INVALIDDEVICE";
	case D3DERR_INVALIDCALL:
		return "INVALIDCALL";
	case D3DERR_DRIVERINVALIDCALL:
		return "DRIVERINVALIDCALL";
	case D3DERR_WASSTILLDRAWING:
		return "WASSTILLDRAWING";
	case D3DOK_NOAUTOGEN:
		return "NOAUTOGEN";
	}
	return "UNKNOWN_ERROR";
}

HRESULT DX_CHECK(HRESULT hr) {
	if (FAILED(hr)) {
		OutputDebugStringA(GetDX9HResult(hr));
		OutputDebugStringA("\n");
	}
	return hr;
}

struct D3D9FrameBuffer
{
	~D3D9FrameBuffer();

	bool create(int width, int height);
	void destory();

	void bind();
	void unbind();

	void setGLHandle(HANDLE glHandle);
	void setD3D9Device(const CComPtr<IDirect3DDevice9Ex>& device);

	int m_width = 0;
	int m_height = 0;

	HANDLE m_glTextureHandles[2] = { 0, 0 };
	GLuint m_glTextures[2] = { 0, 0 };
	GLuint m_glFBO = 0;

	CComPtr<IDirect3DSurface9> m_color;
	CComPtr<IDirect3DSurface9> m_depth;

	HANDLE m_glD3DHandle = 0;
	CComPtr<IDirect3DDevice9Ex> m_pd3dDevice;
};

D3D9FrameBuffer::~D3D9FrameBuffer()
{
	destory();
}

bool D3D9FrameBuffer::create(int width, int height)
{
	const bool lockable = false;

	// D3DMULTISAMPLE_4_SAMPLES - 활성화 시, 렌더버퍼의 기존 화면이 유지되지 않음. 검은색(clear 지정색과 관계 없음)으로 초기화
	// D3DMULTISAMPLE_NONE - 선택시, 기존 렌더 버퍼의 화면 유지됨
	DX_CHECK(m_pd3dDevice->CreateRenderTarget(
		width, height, D3DFMT_A8R8G8B8,
		D3DMULTISAMPLE_NONE, 0, lockable,
		&m_color, NULL));

	DX_CHECK(m_pd3dDevice->CreateDepthStencilSurface(
		width, height, D3DFMT_D24S8,
		D3DMULTISAMPLE_NONE, 0, lockable,
		&m_depth, NULL));

	glGenFramebuffers(1, &m_glFBO);
	glGenTextures(2, m_glTextures);

	m_glTextureHandles[0] = wglDXRegisterObjectNV(m_glD3DHandle, m_color,
		m_glTextures[0], GL_TEXTURE_2D,
		WGL_ACCESS_WRITE_DISCARD_NV);

	assert(m_glTextureHandles[0] != 0);

	m_glTextureHandles[1] = wglDXRegisterObjectNV(m_glD3DHandle, m_depth,
		m_glTextures[1], GL_TEXTURE_2D,
		WGL_ACCESS_WRITE_DISCARD_NV);
	assert(m_glTextureHandles[1] != 0);

	// undocumented / needed for frmaebuffer_complete
	// https://github.com/sharpdx/SharpDX/blob/master/Source/SharpDX.Direct3D9/Surface.cs
	wglDXLockObjectsNV(m_glD3DHandle, 2, m_glTextureHandles);

	glBindFramebuffer(GL_FRAMEBUFFER, m_glFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, m_glTextures[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, m_glTextures[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
		GL_TEXTURE_2D, m_glTextures[1], 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	wglDXUnlockObjectsNV(m_glD3DHandle, 2, m_glTextureHandles);

	return true;
}

void D3D9FrameBuffer::destory()
{
	assert(m_glD3DHandle != 0);

	if (m_glTextureHandles[0] != 0) {
		wglDXUnregisterObjectNV(m_glD3DHandle, m_glTextureHandles[0]);
		m_glTextureHandles[0] = 0;
	}
	if (m_glTextureHandles[1] != 0) {
		wglDXUnregisterObjectNV(m_glD3DHandle, m_glTextureHandles[1]);
		m_glTextureHandles[1] = 0;
	}
	m_color = NULL;
	m_depth = NULL;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &m_glFBO);
	m_glFBO = 0;

	glDeleteTextures(2, m_glTextures);
	m_glTextures[0] = 0;
	m_glTextures[1] = 0;

	m_glD3DHandle = 0;
	m_pd3dDevice = NULL;
}

void D3D9FrameBuffer::bind()
{
	assert(m_color != nullptr);

	// lock the render targets for GL access
	wglDXLockObjectsNV(m_glD3DHandle, 2, m_glTextureHandles);

	glBindFramebuffer(GL_FRAMEBUFFER, m_glFBO);
}

void D3D9FrameBuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// unlock the render targets
	wglDXUnlockObjectsNV(m_glD3DHandle, 2, m_glTextureHandles);
}

void D3D9FrameBuffer::setGLHandle(HANDLE glHandle)
{
	m_glD3DHandle = glHandle;
}

void D3D9FrameBuffer::setD3D9Device(const CComPtr<IDirect3DDevice9Ex>& device)
{
	m_pd3dDevice = device;
}

class TriangleRender;

using D3D9FrameBufferPtr = std::shared_ptr<struct D3D9FrameBuffer>;

class D3D9Driver
{
public:

	~D3D9Driver();

	bool create();
	void destroy();

	void resize(int width, int height);
	void makeCurrent();

	D3D9FrameBufferPtr createFrameBuffer(int width, int height);

	HWND m_hWnd = 0;

	std::shared_ptr<WGLContext> m_wglContext;

	D3DPRESENT_PARAMETERS m_d3dpp;

	HANDLE m_glD3DHandle = 0;

	CComPtr<IDirect3D9Ex> m_pD3D;
	CComPtr<IDirect3DDevice9Ex> m_pd3dDevice;
};

D3D9Driver::~D3D9Driver()
{
	destroy();
}

bool D3D9Driver::create()
{
	m_hWnd = CreateWindowA("STATIC", "dummy", 0, 0, 0, 100, 100, 0, 0, 0, 0);
	assert(m_hWnd);

	/*
	 * IDirect3D9Ex device is required.
	 *
	 * if not, wglDXRegisterObjectNV returns 0
	 */
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D)))
		return false;

	// https://docs.microsoft.com/en-us/windows/win32/direct3d9/full-scene-antialiasing
	/*
	 * The code below assumes that pD3D is a valid pointer
	 *   to a IDirect3D9 interface.
	 */
	DWORD quality;
	if (FAILED(m_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, TRUE,
		D3DMULTISAMPLE_4_SAMPLES, &quality)))
		return false;

	// Set up the structure used to create the D3DDevice
	ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
	m_d3dpp.Windowed = TRUE;
	m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; // required. to enable multisampling
	m_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	m_d3dpp.EnableAutoDepthStencil = TRUE;
	m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;

	// Create the D3DDevice
	HRESULT hr = S_OK;
	if (FAILED(hr = (m_pD3D->CreateDeviceEx(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING |
		D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED,
		&m_d3dpp, NULL, &m_pd3dDevice))))
	{
		return false;
	}

	auto wglContext = std::make_shared<WGLContext>();
	if (!wglContext->create(m_hWnd))
		return false;
	if (GLAD_WGL_NV_DX_interop == 0)
		return false;
	m_wglContext = wglContext;

	// requires current wglContext
	assert(m_wglContext);

	// register the Direct3D device with GL
	m_glD3DHandle = wglDXOpenDeviceNV(m_pd3dDevice);
	assert(m_glD3DHandle);

	return true;
}

void D3D9Driver::destroy()
{
	wglDXCloseDeviceNV(m_glD3DHandle);
	m_glD3DHandle = 0;

	m_pd3dDevice = NULL;
	m_pD3D = NULL;

	DestroyWindow(m_hWnd);
	m_hWnd = 0;
}

void D3D9Driver::resize(int width, int height)
{
	if (m_pd3dDevice == nullptr)
		return;

	// device reset error when pixel area is zero
	if (width == 0 || height == 0)
		return;

	if (width == m_d3dpp.BackBufferWidth && height == m_d3dpp.BackBufferHeight)
		return;

	m_d3dpp.BackBufferWidth = width;
	m_d3dpp.BackBufferHeight = height;

	// DeviceEx 에서는 ResetEx 호출 전 리소스 해제 할 필요가 없다
	// 화면 surface 크기 갱신
	DX_CHECK(m_pd3dDevice->ResetEx(&m_d3dpp, NULL));
}

void D3D9Driver::makeCurrent()
{
	m_wglContext->makeCurrent();
}

D3D9FrameBufferPtr D3D9Driver::createFrameBuffer(int width, int height)
{
	auto fbo = std::make_shared<D3D9FrameBuffer>();
	if (!fbo)
		return nullptr;

	fbo->setD3D9Device(m_pd3dDevice);
	fbo->setGLHandle(m_glD3DHandle);

	if (!fbo->create(width, height))
		return nullptr;

	return fbo;
}

D3D9Driver* getOrCreateDriver()
{
	static std::unique_ptr<D3D9Driver> s_driver;
	if (s_driver == nullptr) {
		auto driver = std::make_unique<D3D9Driver>();
		if (driver->create()) {
			s_driver = std::move(driver);
		}
	}
	return s_driver.get();
}

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
	if (m_fbo)
		return m_fbo->m_color;
	return nullptr;
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

