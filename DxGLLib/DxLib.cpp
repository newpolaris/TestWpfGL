#include "DxLib.h"

#include <Windows.h>
#include <d3d9.h>

#pragma comment(lib, "d3d9.lib")

struct DxGLImageImpl
{
	~DxGLImageImpl();

	bool create();
	void destroy();

	IDirect3D9Ex* m_pD3D = NULL;
	IDirect3DDevice9Ex* m_pd3dDevice = NULL;

	D3DPRESENT_PARAMETERS m_d3dpp;
};

DxGLImageImpl::~DxGLImageImpl()
{
}

void DxGLImageImpl::destroy()
{
}

bool DxGLImage::create()
{
	return true;
}

