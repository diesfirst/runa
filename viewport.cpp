#include "viewport.hpp"

Viewport::Viewport(const int width, const int height, Context context) :
	window(width, height),
	swapchain(context, window)
{
}

Viewport::~Viewport()
{
}

