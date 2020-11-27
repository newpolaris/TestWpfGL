#pragma once

struct DxGLRenderImpl;

class DxGLRender
{
public:

	DxGLRender();
	~DxGLRender();

	bool create();
	void destroy();
	void resize(int width, int height);
	void render();

	void* getBackBuffer() const;

	DxGLRenderImpl* m_pImpl = nullptr;
};
