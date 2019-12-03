#include "viewport.hpp"

Viewport::Viewport(const Context& context, const uint16_t width, const uint16_t height) :
	context(context),
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
	window.open();
}

Viewport::~Viewport()
{
}

void Viewport::present()
{
	uint8_t index = swapchain.getCurrentIndex();
	context.pCommander->presentSwapImage(swapchain.swapchain, index);
	swapchain.incrementIndex();
}

uint32_t Viewport::acquireSwapImageIndex(vk::Semaphore semaphore)
{
	return swapchain.acquireNextImage(semaphore);
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
	return swapchain.getImageCount();
}

Swapchain& Viewport::getSwapchain()
{
	return swapchain;
}

vk::SwapchainKHR* Viewport::getPSwapchain()
{
	return &swapchain.swapchain;
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
