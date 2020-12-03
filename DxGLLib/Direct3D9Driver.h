#pragma once

#include <memory>
#include <atlcomcli.h> // CComPtr
#include <d3d9.h>
#include <glad/glad.h>
#include <WGLContext.h>

using D3D9FrameBufferPtr = std::shared_ptr<struct D3D9FrameBuffer>;

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

D3D9Driver* getOrCreateDriver();

