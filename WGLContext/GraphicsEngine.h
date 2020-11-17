#pragma once

class GraphicsEngineImpl;
class GraphicsEngine
{
public:

	GraphicsEngine();
	~GraphicsEngine();

	bool create(int width, int height, bool bVisible);
	void destroy();
	void render();
	void renderToBuffer(char* imageBuffer);
	void resize(int width, int height);

	GraphicsEngineImpl* pImpl = nullptr;
};
