#pragma once

struct DxGLRenderImpl;

class DxGLRender
{
public:

	DxGLRender();
	~DxGLRender();

	DxGLRenderImpl* m_pImpl = nullptr;
};
