#include "viewport.hpp"

Viewport::Viewport(Context& context ,const int width, const int height) :
	window(width, height),
	swapchain(context, window),
	viewport(0, 0, width, height, 0, 1) //need to look up what these args do
{
	extent = vk::Extent2D(width, height);
	this->width = width;
	this->height = height;
	window.open();
}

Viewport::~Viewport()
{
}

vk::Extent2D Viewport::getExtent() const
{
	return extent;
}

uint32_t Viewport::getWidth() const
{
	return width;
}

uint32_t Viewport::getHeight() const
{
	return height;
}

uint8_t Viewport::getSwapImageCount() const
{
	return swapchain.imageCount;
}

const std::vector<vk::ImageView>& Viewport::getSwapImageViews() const
{
	return swapchain.imageViews;
}

vk::ImageView Viewport::getSwapImageView(uint32_t i) const
{
	return swapchain.imageViews[i];
}

