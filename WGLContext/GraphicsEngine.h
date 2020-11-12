#pragma once

#include "TriangleRenderer.h"

struct GLFWwindow;
class TriangleRender;

class GraphicsEngine
{
public:

	~GraphicsEngine();

	bool create(int width, int height);
	void destroy();
	void render();
	void resize(int width, int height);

	GLFWwindow* m_window = nullptr;
	TriangleRender* m_triangle = nullptr;
};

