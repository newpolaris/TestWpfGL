// NV_DX_interop  : https://www.khronos.org/registry/OpenGL/extensions/NV/WGL_NV_DX_interop.txt
// NV_DX_interop2 : https://www.khronos.org/registry/OpenGL/extensions/NV/WGL_NV_DX_interop2.txt
// mmonzeiko's    : https://gist.github.com/mmozeiko/c99f9891ce723234854f0919bfd88eae

#include <Windows.h>
#include <d3d11.h>

#include <glad/glad.h>
#include <glad/glad_wgl.h>

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

static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
}

static void Create(HWND window)
{
    // GL context on temporary window, no drawing will happen to this window
    {
        temp = CreateWindowA("STATIC", "temp", 0, 0, 0, 0, 0, 0, 0, 0, 0);
        Assert(temp);

        tempdc = GetDC(temp);
        Assert(tempdc);

        PIXELFORMATDESCRIPTOR pfd =
        {
            .nSize = sizeof(pfd),
            .nVersion = 1,
            .dwFlags = PFD_SUPPORT_OPENGL,
            .iPixelType = PFD_TYPE_RGBA,
            .iLayerType = PFD_MAIN_PLANE,
        };

        int format = ChoosePixelFormat(tempdc, &pfd);
        Assert(format);

        DescribePixelFormat(tempdc, format, sizeof(pfd), &pfd);
        BOOL set = SetPixelFormat(tempdc, format, &pfd);
        Assert(set);

        temprc = wglCreateContext(tempdc);
        Assert(temprc);

        BOOL make = wglMakeCurrent(tempdc, temprc);
        Assert(make);

        auto wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

        int attrib[] =
        {
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            0,
        };

        HGLRC newrc = wglCreateContextAttribsARB(tempdc, NULL, attrib);
        Assert(newrc);

        make = wglMakeCurrent(tempdc, newrc);
        Assert(make);

        wglDeleteContext(temprc);
        temprc = newrc;

		if (!gladLoadGL())
			Assert(false);
		if (!gladLoadWGL(tempdc))
			Assert(false);

        Assert(GLAD_WGL_NV_DX_interop2 != 0);
    }

	glDebugMessageCallback(DebugCallback, 0);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

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

static void Destroy()
{
    context->ClearState();

    wglDXUnregisterObjectNV(dxDevice, dxColor);
    wglDXUnregisterObjectNV(dxDevice, dxDepthStencil);

    glDeleteFramebuffers(1, &fbuf);
    glDeleteRenderbuffers(1, &colorRbuf);
    glDeleteRenderbuffers(1, &dsRbuf);

    wglDXCloseDeviceNV(dxDevice);

    wglMakeCurrent(tempdc, NULL);
    wglDeleteContext(temprc);
    ReleaseDC(temp, tempdc);

    colorView->Release();
    dsView->Release();
    context->Release();
    device->Release();
    swapChain->Release();

    colorView = 0;
    dsView = 0;
    context = 0;
    device = 0;
    swapChain = 0;
}

static void Resize(int width, int height)
{
    HRESULT hr;

    if (colorView)
    {
        wglDXUnregisterObjectNV(dxDevice, dxColor);
        wglDXUnregisterObjectNV(dxDevice, dxDepthStencil);

        context->OMSetRenderTargets(0, NULL, NULL);

        colorView->Release();
        dsView->Release();

        colorView = 0;
        dsView = 0;

        hr = swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_UNKNOWN, 0);
        AssertHR(hr);
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

    context->RSSetViewports(1, {&view});

    glViewport(0, 0, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dsRbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dsRbuf);
}

LRESULT CALLBACK WndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_CREATE:
        Create(window);
        return 0;

    case WM_DESTROY:
        Destroy();
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        Resize(LOWORD(lparam), HIWORD(lparam));
        return 0;
    }
    return DefWindowProcA(window, msg, wparam, lparam);
}

int main()
{
    WNDCLASSA wc = {0, };
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
            if (msg.message == WM_QUIT)
            {
                running = 0;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (!running)
        {
            break;
        }

        // render with D3D
        {
            FLOAT cornflowerBlue[] = {100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.f};
            context->OMSetRenderTargets(1, &colorView, dsView);
            context->ClearRenderTargetView(colorView, cornflowerBlue);
            context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0, 0);
        }

        HANDLE dxObjects[] = {dxColor, dxDepthStencil};
        wglDXLockObjectsNV(dxDevice, _countof(dxObjects), dxObjects);

        // render with GL
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fbuf);

            glBegin(GL_TRIANGLES);
            glColor3f(1, 0, 0);
            glVertex2f(0.f, -0.5f);
            glColor3f(0, 1, 0);
            glVertex2f(0.5f, 0.5f);
            glColor3f(0, 0, 1);
            glVertex2f(-0.5f, 0.5f);
            glEnd();

            glBindFramebuffer(GL_FRAMEBUFFER, fbuf);
        }

        wglDXUnlockObjectsNV(dxDevice, _countof(dxObjects), dxObjects);
        HRESULT hr = swapChain->Present(1, 0);
        Assert(SUCCEEDED(hr));
    }
    return 0;
}
