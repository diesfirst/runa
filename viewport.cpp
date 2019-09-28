#include "viewport.hpp"

Viewport::Viewport(Context& context, const uint16_t width, const uint16_t height) :
	window(width, height),
	swapchain(context, window, SWAP_IMG_COUNT), //set to 3 swapchain images
	viewport(0, 0, width, height, 0, 1) //need to look up what these args do
{
	extent = vk::Extent2D(width, height);
	this->width = width;
	this->height = height;
	scissor.setExtent(extent);
	scissor.setOffset({0, 0});
	state.setPScissors(&scissor);
	state.setPViewports(&viewport);
	state.setScissorCount(1);
	state.setViewportCount(1);
	swapchainImageCount = SWAP_IMG_COUNT;
	context.pCommander->allocateCommandBuffers(SWAP_IMG_COUNT);
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

Swapchain& Viewport::getSwapchain()
{
	return swapchain;
}

const std::vector<vk::ImageView>& Viewport::getSwapImageViews() const
{
	return swapchain.imageViews;
}

vk::ImageView Viewport::getSwapImageView(uint32_t i) const
{
	return swapchain.imageViews[i];
}

vk::PipelineViewportStateCreateInfo* Viewport::getPViewportState()
{
	return &state;
}
