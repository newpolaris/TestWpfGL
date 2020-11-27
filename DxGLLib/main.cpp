// https://www.khronos.org/registry/OpenGL/extensions/NV/WGL_NV_DX_interop.txt
// https://github.com/markkilgard/NVprSDK/blob/master/nvpr_examples/nvpr_dx9/nvpr_dx9.cpp
// https://github.com/tliron/opengl-3d-vision-bridge/blob/master/opengl_3dv.c
// https://github.com/bkaradzic/bgfx/blob/master/src/renderer_d3d9.cpp

#include <Windows.h>
#include <d3d9.h>
#include <cmath>
#include <stdio.h>
#include <cassert>
#include <glad/glad.h>
#include <glad/glad_wgl.h>

#include <WGLContext.h>

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

class DXGLRender
{
public:

	void onPreReset();
	bool onPostReset();
	void resize(WPARAM wParam, LPARAM lParam);
	void render();

	bool initialize(HWND hWnd);
	void cleanup();

	IDirect3D9Ex* m_pD3D = NULL; // Used to create the D3DDevice
	IDirect3DDevice9Ex* m_pd3dDevice = NULL; // Our rendering device
	IDirect3DSwapChain9* m_swapChain = NULL;
	IDirect3DSurface9* m_backBufferColor = NULL;
	IDirect3DSurface9* m_backBufferDepthStencil = NULL;

	// create the Direct3D render targets
	IDirect3DSurface9* m_dxColorBuffer = NULL;
	IDirect3DSurface9* m_dxDepthBuffer = NULL;

	D3DPRESENT_PARAMETERS m_d3dpp;

	HWND m_hWnd = 0;

	WGLContext m_glContext;

	HANDLE m_glD3DHandle = 0;
	HANDLE m_glTextureHandles[2] = { 0, 0 };
	GLuint m_glTextures[2] = { 0, 0 };
	GLuint m_glFBO = 0;
};

static HWND tempHwnd;

void DXGLRender::onPreReset()
{
	if (m_glTextureHandles[0] != 0) {
		wglDXUnregisterObjectNV(m_glD3DHandle, m_glTextureHandles[0]);
		wglDXUnregisterObjectNV(m_glD3DHandle, m_glTextureHandles[1]);
		m_glTextureHandles[0] = 0;
		m_glTextureHandles[1] = 0;
	}
	if (m_backBufferColor) {
		m_backBufferColor->Release();
		m_backBufferColor = NULL;
	}
	if (m_backBufferDepthStencil) {
		m_backBufferDepthStencil->Release();
		m_backBufferDepthStencil = NULL;
	}
	if (m_swapChain) {
		m_swapChain->Release();
		m_swapChain = NULL;
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

bool DXGLRender::onPostReset()
{
	// swapchain은 ResetEx 여부와 관계 없이 유지되는 듯
	DX_CHECK(m_pd3dDevice->GetSwapChain(0, &m_swapChain));
	// ResetEx 이후 갱신 필요함
	DX_CHECK(m_swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_backBufferColor));
	DX_CHECK(m_pd3dDevice->GetDepthStencilSurface(&m_backBufferDepthStencil));

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

void DXGLRender::cleanup()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &m_glFBO);
	m_glFBO = 0;

	glDeleteTextures(2, m_glTextures);
	m_glTextures[0] = 0;
	m_glTextures[1] = 0;

	wglDXCloseDeviceNV(m_glD3DHandle);
	m_glD3DHandle = 0;

	onPreReset();

	if (m_pd3dDevice != NULL) {
		m_pd3dDevice->Release();
		m_pd3dDevice = NULL;
	}
	if (m_pD3D != NULL) {
		m_pD3D->Release();
		m_pD3D = NULL;
	}

	m_glContext.destory();

	DestroyWindow(m_hWnd);
	m_hWnd = 0;
}

// https://github.com/bkaradzic/bgfx/blob/master/src/renderer_d3d9.cpp
// Introduction to Directx 9.0c 's sample

void DXGLRender::resize(WPARAM wParam, LPARAM lParam)
{
	if (m_pd3dDevice == nullptr)
		return;

	D3DDEVICE_CREATION_PARAMETERS dcp;
	DX_CHECK(m_pd3dDevice->GetCreationParameters(&dcp));

	D3DDISPLAYMODE dm;
	DX_CHECK(m_pD3D->GetAdapterDisplayMode(dcp.AdapterOrdinal, &dm));

	m_d3dpp.BackBufferWidth = LOWORD(lParam);
	m_d3dpp.BackBufferHeight = HIWORD(lParam);

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

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
}

bool DXGLRender::initialize(HWND hWnd)
{
	m_hWnd = CreateWindowA("STATIC", "temp", 0, 0, 0, 0, 0, 0, 0, 0, 0);
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
	if (FAILED(m_pD3D->CreateDeviceEx(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING |
		D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED,
		&m_d3dpp, NULL, &m_pd3dDevice)))
	{
		return false;
	}

	if (!m_glContext.create(m_hWnd))
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

void DXGLRender::render()
{
	assert(m_dxColorBuffer != nullptr);
	{
		// lock the render targets for GL access
		wglDXLockObjectsNV(m_glD3DHandle, 2, m_glTextureHandles);

		glBindFramebuffer(GL_FRAMEBUFFER, m_glFBO);
		{
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
			glShadeModel(GL_SMOOTH);
			glBegin(GL_TRIANGLES);
			glColor3f(1, 0, 0);
			glVertex2f(0.f, -0.5f);
			glColor3f(0, 1, 0);
			glVertex2f(0.5f, 0.5f);
			glColor3f(0, 0, 1);
			glVertex2f(-0.5f, 0.5f);
			glEnd();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// unlock the render targets
		wglDXUnlockObjectsNV(m_glD3DHandle, 2, m_glTextureHandles);
	}

	// just set back to screen surface
	m_pd3dDevice->SetRenderTarget(0, m_backBufferColor);

	// copy rendertarget to backbuffer
	m_pd3dDevice->StretchRect(m_dxColorBuffer, NULL, m_backBufferColor, NULL, D3DTEXF_NONE);

	// https://stackoverflow.com/questions/61915988/how-to-handle-direct3d-9ex-d3derr-devicehung-error
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9ex-checkdevicestate
	// We recommend not to call CheckDeviceState every frame. Instead, call CheckDeviceState only 
	// if the IDirect3DDevice9Ex::PresentEx method returns a failure code.
	HRESULT hr = m_pd3dDevice->PresentEx(NULL, NULL, NULL, NULL, 0);
	if (FAILED(hr))
	{
		onPreReset();

		hr = m_pd3dDevice->CheckDeviceState(nullptr);

		// 복잡한 예외 처리 방법은 아래 참조:
		// https://github.com/google/angle/blob/master/src/libANGLE/renderer/d3d/d3d9/Renderer9.cpp
		for (int attempts = 5; attempts > 0; attempts--)
		{
			// TODO: Device removed, which may trigger on driver reinstallation
			assert(hr != D3DERR_DEVICEREMOVED);

			Sleep(500);
			DX_CHECK(m_pd3dDevice->ResetEx(&m_d3dpp, NULL));
			if (SUCCEEDED(m_pd3dDevice->CheckDeviceState(nullptr)))
				break;
		}

		onPostReset();
	}
}

DXGLRender dxgl;

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
		dxgl.render();
		break;

	case WM_SIZE:
		dxgl.resize(wParam, lParam);
		return 0;
	case WM_DESTROY:
		dxgl.cleanup();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int main()
{
	// Register the window class
	WNDCLASSEX wc =
	{
		// https://stackoverflow.com/a/32806642/1890382
		sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_OWNDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"D3D Tutorial", NULL
	};
	RegisterClassEx(&wc);

	// Create the application's window
	HWND hWnd = CreateWindow(L"D3D Tutorial", L"D3D Tutorial 02: Vertices",
		WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
		NULL, NULL, wc.hInstance, NULL);

	if (SUCCEEDED(dxgl.initialize(hWnd)))
	{
		// Create the vertex buffer
		if (TRUE)
		{
			// Show the window
			ShowWindow(hWnd, SW_SHOWDEFAULT);
			UpdateWindow(hWnd);

			// Enter the message loop
			MSG msg;
			ZeroMemory(&msg, sizeof(msg));
			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else {
					dxgl.render();
				}
			}
		}
	}

	UnregisterClass(L"D3D Tutorial", wc.hInstance);

	return 0;
}
