// https://gist.github.com/mmozeiko/c99f9891ce723234854f0919bfd88eae
// mmonzeiko / dxgl.c

#include <Windows.h>
#include <d3d11.h>
#include <GL/GL.h>
#include <GL/glext.h>
#include <GL/wglext.h>

// #include <glad/glad.h>
// #include <glad/glad_wgl.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)
#define AssertHR(hr) Assert(SUCCEEDED(hr))

static HWND temp;
static HDC tempdc;
static HGLRC temprc;

static GLuint colorRbuf;
static GLuint dsRbuf;
static GLuint fbuf;

static HANDLE dxDevice;
static HANDLE dxColor;
static HANDLE dxDepthStencil;

static ID3D11Device* device;
static ID3D11DeviceContext* context;
static IDXGISwapChain* swapChain;
static ID3D11RenderTargetView* colorView;
static ID3D11DepthStencilView* dsView;

static PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV;
static PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV;
static PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV;
static PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV;
static PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV;
static PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV;

static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
static PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
static PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
static PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;

static void Create(HWND window)
{
	// GL context on temporary window, no drawing will happen to this window
	{
		temp = CreateWindowA("STATIC", "temp", 0, 0, 0, 0, 0, 0, 0, 0, 0);
		Assert(temp);

		tempdc = GetDC(temp);
		Assert(tempdc);

		PIXELFORMATDESCRIPTOR pfd = { 0, };
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_SUPPORT_OPENGL;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int format = ChoosePixelFormat(tempdc, &pfd);
		Assert(format);

		DescribePixelFormat(tempdc, format, sizeof(pfd), &pfd);
		BOOL set = SetPixelFormat(tempdc, format, &pfd);
		Assert(set);

		temprc = wglCreateContext(tempdc);
		Assert(temprc);

		BOOL make = wglMakeCurrent(tempdc, temprc);
        Assert(make);
	}

    wglDXOpenDeviceNV = (PFNWGLDXOPENDEVICENVPROC)wglGetProcAddress("wglDXOpenDeviceNV");
    wglDXCloseDeviceNV = (PFNWGLDXCLOSEDEVICENVPROC)wglGetProcAddress("wglDXCloseDeviceNV");

    wglDXRegisterObjectNV = (PFNWGLDXREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXRegisterObjectNV");
    wglDXUnregisterObjectNV = (PFNWGLDXUNREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXUnregisterObjectNV");

    wglDXLockObjectsNV = (PFNWGLDXLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXLockObjectsNV");
    wglDXUnlockObjectsNV = (PFNWGLDXUNLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXUnlockObjectsNV");

    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");

    glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
    glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");

    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
    glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");

	RECT rect;
	GetClientRect(window, &rect);
	int width = rect.right = rect.left;
	int height = rect.bottom - rect.top;

	// TODO: compare swapchain desc with others

	DXGI_SWAP_CHAIN_DESC desc = {
		.BufferDesc =
		{
			.RefreshRate = 
			{
				.Numerator = 60,
				.Denominator = 1,
			},
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		},
		.SampleDesc =
		{
			.Count = 1,
			.Quality = 0,
		},
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = 1,
		.OutputWindow = window,
		.Windowed = TRUE,
		.SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
		.Flags = 0,
	};

	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, 0,
		D3D11_SDK_VERSION, &desc, &swapChain, &device, NULL, &context);
	AssertHR(hr);

	dxDevice = wglDXOpenDeviceNV(device);
	Assert(dxDevice);

	glGenRenderbuffers(1, &colorRbuf);
	glGenRenderbuffers(1, &dsRbuf);
	glGenFramebuffers(1, &fbuf);
	glBindFramebuffer(GL_FRAMEBUFFER, fbuf);
}

static void Resize(int width, int height)
{
	HRESULT hr;

	if (colorView)
	{
		wglDXUnregisterObjectNV(dxDevice, dxColor);
		wglDXUnregisterObjectNV(dxDevice, dxDepthStencil);
	}

	ID3D11Texture2D* colorBuffer;
	hr = swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&colorBuffer);
	AssertHR(hr);

	hr = device->CreateRenderTargetView(colorBuffer, NULL, &colorView);
	AssertHR(hr);

    D3D11_TEXTURE2D_DESC desc =
    {
        .Width = (UINT)width,
        .Height = (UINT)height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        .SampleDesc =
        {
            .Count = 1,
            .Quality = 0,
        },
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_DEPTH_STENCIL,
        .CPUAccessFlags = 0,
        .MiscFlags = 0,
    };

    ID3D11Texture2D* dsBuffer;
    hr = device->CreateTexture2D(&desc, NULL, &dsBuffer);
    AssertHR(hr);

	hr = device->CreateDepthStencilView(dsBuffer, NULL, &dsView);
	AssertHR(hr);

	dxColor = wglDXRegisterObjectNV(dxDevice, colorBuffer, colorRbuf, GL_RENDERBUFFER, WGL_ACCESS_READ_WRITE_NV);
	Assert(dxColor);

	dxDepthStencil = wglDXRegisterObjectNV(dxDevice, dsBuffer, dsRbuf, GL_RENDERBUFFER, WGL_ACCESS_READ_WRITE_NV);
	Assert(dxDepthStencil);

	colorBuffer->Release();
	colorBuffer = 0;

	dsBuffer->Release();
	dsBuffer = 0;

	D3D11_VIEWPORT view =
	{
		.TopLeftX = 0.f,
		.TopLeftY = 0.f,
		.Width = (float)width,
		.Height = (float)height,
		.MinDepth = 0.f,
		.MaxDepth = 1.f,
	};

	context->RSSetViewports(1, { &view });

    glViewport(0, 0, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dsRbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dsRbuf);
}

LRESULT CALLBACK WndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg)
	{
	case WM_CREATE:
        Create(window);
		return 0;

	case WM_SIZE:
		Resize(LOWORD(lparam), HIWORD(lparam));
		return 0;
	}
	return DefWindowProcA(window, msg, wparam, lparam);
}

int main()
{
	WNDCLASSA wc = { 0, };
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "DXGL";

	ATOM atom = RegisterClassA(&wc);
	Assert(atom);

	HWND window = CreateWindowA(wc.lpszClassName, "DXGL",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
	Assert(window);

	int running = 1;
	for (;;)
	{
		MSG msg;
		while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		// render with D3D
		{
			FLOAT cornflowerBlue[] = { 100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.f };
			context->OMSetRenderTargets(1, &colorView, dsView);
			context->ClearRenderTargetView(colorView, cornflowerBlue);
			context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0, 0);
		}

		HANDLE dxObjects[] = { dxColor, dxDepthStencil };
		wglDXLockObjectsNV(dxDevice, _countof(dxObjects), dxObjects); 
		wglDXUnlockObjectsNV(dxDevice, _countof(dxObjects), dxObjects);
        HRESULT hr = swapChain->Present(1, 0);
        Assert(SUCCEEDED(hr));
	}
	return 0;
}