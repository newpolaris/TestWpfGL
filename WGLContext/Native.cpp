#include "Native.h"
#include "WGLContext.h"
#include "TriangleRenderer.h"

namespace {
	WGLContext* context = nullptr;
	TriangleRender* render = nullptr;
}

extern "C" {

	int GLCreate(const void* handle)
	{
		context = new WGLContext();
		if (!context->create((HWND)handle))
			return -1;

		render = new TriangleRender();
		if (!render->create())
			return -1;

		return 0;
	}

	int Render()
	{
		if (context) {
			context->makeCurrent();
			if (render)
				render->draw();
			context->present();
		}
		return 0;
	}
}

