#include "DxLib.h"

#include <Windows.h>
#include <d3d9.h>

#include <glad/glad.h>
#include <glad/glad_wgl.h>
#include <cassert>

#include <WGLContext.h>
#include <TriangleRenderer.h>

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
		DebugBreak();
	}
	return hr;
}

static WGLContext* g_glContext = nullptr;

class TriangleRender;

struct DxGLRenderImpl
{
public:

	void onPreReset();
	bool onPostReset();
	void resize(int width, int height);
	void render();

	bool create();
	void destroy();

	IDirect3D9Ex* m_pD3D = NULL; // Used to create the D3DDevice
	IDirect3DDevice9Ex* m_pd3dDevice = NULL; // Our rendering device

	// create the Direct3D render targets
	IDirect3DSurface9* m_dxColorBuffer = NULL;
	IDirect3DSurface9* m_dxDepthBuffer = NULL;

	D3DPRESENT_PARAMETERS m_d3dpp;

	HWND m_hWnd = 0;

	HANDLE m_glD3DHandle = 0;
	HANDLE m_glTextureHandles[2] = { 0, 0 };
	GLuint m_glTextures[2] = { 0, 0 };
	GLuint m_glFBO = 0;

	TriangleRender* m_triangle = nullptr;
};

void DxGLRenderImpl::onPreReset()
{
	assert(m_glD3DHandle != 0);

	g_glContext->makeCurrent();

	if (m_glTextureHandles[0] != 0 || m_glTextureHandles[1] != 0) {
		wglDXUnregisterObjectNV(m_glD3DHandle, m_glTextureHandles[0]);
		wglDXUnregisterObjectNV(m_glD3DHandle, m_glTextureHandles[1]);
		m_glTextureHandles[0] = 0;
		m_glTextureHandles[1] = 0;
	}
	if (m_dxColorBuffer) {
		m_dxColorBuffer->Release();
		m_dxColorBuffer = NULL;
	}
	if (m_dxDepthBuffer) {
		m_dxDepthBuffer->Release();
		m_dxDepthBuffer = NULL;
	}
}

bool DxGLRenderImpl::onPostReset()
{
	g_glContext->makeCurrent();

	UINT width = m_d3dpp.BackBufferWidth;
	UINT height = m_d3dpp.BackBufferHeight;
	bool lockable = false;

	// D3DMULTISAMPLE_4_SAMPLES - 활성화 시, 렌더버퍼의 기존 화면이 검은색(clear 지정색과 관계 없음)으로 초기화
	// D3DMULTISAMPLE_NONE - 선택시, 기존 렌더 버퍼의 화면 유지됨
	DX_CHECK(m_pd3dDevice->CreateRenderTarget(
		width, height, D3DFMT_A8R8G8B8,
		D3DMULTISAMPLE_NONE, 0, lockable,
		&m_dxColorBuffer, NULL));

	DX_CHECK(m_pd3dDevice->CreateDepthStencilSurface(
		width, height, D3DFMT_D24S8,
		D3DMULTISAMPLE_NONE, 0, lockable,
		&m_dxDepthBuffer, NULL));

	m_glTextureHandles[0] = wglDXRegisterObjectNV(m_glD3DHandle, m_dxColorBuffer,
		m_glTextures[0], GL_TEXTURE_2D,
		WGL_ACCESS_WRITE_DISCARD_NV);

	assert(m_glTextureHandles[0] != 0);

	m_glTextureHandles[1] = wglDXRegisterObjectNV(m_glD3DHandle, m_dxDepthBuffer,
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

void DxGLRenderImpl::destroy()
{
	g_glContext->makeCurrent();

	if (m_triangle) {
		m_triangle->destroy();
		delete m_triangle;
		m_triangle = nullptr;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &m_glFBO);
	m_glFBO = 0;

	glDeleteTextures(2, m_glTextures);
	m_glTextures[0] = 0;
	m_glTextures[1] = 0;

	onPreReset();

	wglDXCloseDeviceNV(m_glD3DHandle);
	m_glD3DHandle = 0;

	if (m_pd3dDevice != NULL) {
		m_pd3dDevice->Release();
		m_pd3dDevice = NULL;
	}
	if (m_pD3D != NULL) {
		m_pD3D->Release();
		m_pD3D = NULL;
	}

	// g_glContext->destory();

	DestroyWindow(m_hWnd);
	m_hWnd = 0;
}

// https://github.com/bkaradzic/bgfx/blob/master/src/renderer_d3d9.cpp
// Introduction to Directx 9.0c 's sample

void DxGLRenderImpl::resize(int width, int height)
{
	if (m_pd3dDevice == nullptr)
		return;

	if (m_d3dpp.BackBufferWidth == width && 
		m_d3dpp.BackBufferHeight == height)
		return;

	g_glContext->makeCurrent();

	D3DDEVICE_CREATION_PARAMETERS dcp;
	DX_CHECK(m_pd3dDevice->GetCreationParameters(&dcp));

	D3DDISPLAYMODE dm;
	DX_CHECK(m_pD3D->GetAdapterDisplayMode(dcp.AdapterOrdinal, &dm));

	m_d3dpp.BackBufferWidth = width;
	m_d3dpp.BackBufferHeight = height;

	// device reset error when pixel area is zero
	if (m_d3dpp.BackBufferWidth == 0 || m_d3dpp.BackBufferHeight == 0)
		return;

	// DeviceEx 에서는 ResetEx 전 리소스 해제 하지 않았다고, 에러나오지 않는다.
	// DeviceEx 에서는 MANAGED 리소스는 이제 쓸 수 없다
	// 다만, 예전의 구조를 그냥 놓아둠
	onPreReset();
	DX_CHECK(m_pd3dDevice->ResetEx(&m_d3dpp, NULL));
	onPostReset();

	glViewport(0, 0, m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight);
}

static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool DxGLRenderImpl::create()
{
	m_hWnd = CreateWindowA("STATIC", "dummy", 0, 0, 0, 100, 100, 0, 0, 0, 0);
	assert(m_hWnd);

	// System::Windows::Interop::Hw

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

	if (g_glContext == nullptr) {
		g_glContext = new WGLContext();
		if (!g_glContext->create(m_hWnd))
			return false;
	}

	m_triangle = new TriangleRender();
	if (!m_triangle)
		return false;
	if (!m_triangle->create())
		return false;

	// register the Direct3D device with GL
	m_glD3DHandle = wglDXOpenDeviceNV(m_pd3dDevice);
	assert(m_glD3DHandle);

	glGenFramebuffers(1, &m_glFBO);
	glGenTextures(2, m_glTextures);

	// WM_PAINT가 Resize 보다 먼저 호출되므로,
	onPostReset();

	return true;
}

void DxGLRenderImpl::render()
{
	assert(m_dxColorBuffer != nullptr);
	{
		// lock the render targets for GL access
		wglDXLockObjectsNV(m_glD3DHandle, 2, m_glTextureHandles);

		glBindFramebuffer(GL_FRAMEBUFFER, m_glFBO);
		
		m_triangle->draw();
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// unlock the render targets
		wglDXUnlockObjectsNV(m_glD3DHandle, 2, m_glTextureHandles);
	}

	// copy rendertarget to backbuffer
	// m_pd3dDevice->StretchRect(m_dxColorBuffer, NULL, m_backBufferColor, NULL, D3DTEXF_NONE);
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
	return m_pImpl->m_dxColorBuffer;
}
