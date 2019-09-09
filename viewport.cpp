#include "viewport.hpp"

Viewport::Viewport(const int width, const int height, Context& context) :
	window(width, height),
	swapchain(context, window)
{
	extent = vk::Extent2D(width, height);
}

Viewport::~Viewport()
{
}

vk::Extent2D Viewport::getExtent()
{
	return extent;
}

uint32_t Viewport::getWidth()
{
	return width;
}

uint32_t Viewport::getHeight()
{
	return height;
}
