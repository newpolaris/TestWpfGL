#pragma once

#include <Windows.h>

class WGLContext
{
public:

	~WGLContext();

	bool create(HWND hWnd);
	void destory();
	void makeCurrent();
	void present();

	HDC m_hDC = 0;
	HWND m_hWnd = 0;
	HGLRC m_hRC = 0;
};

