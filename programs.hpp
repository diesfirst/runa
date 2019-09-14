#include "viewport.hpp"
#include "renderer.hpp"

void program1()
{
	Context context;
	Viewport viewport(context);
	Renderer renderer(context);
	renderer.bindToViewport(viewport);
}
