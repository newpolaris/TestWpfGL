#include "TriangleRenderer.h"
#include "GraphicsEngine.h"
#include <memory>

int main()
{
	auto window = std::make_unique<GraphicsEngine>();
	if (!window->create(640, 480))
		return -1;

	while (true) 
		window->render();
	window->destroy();
	window = nullptr;

	return 0;
}

