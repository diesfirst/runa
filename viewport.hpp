// Holds a swapchain and a window. 
#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "swapchain.hpp"
#include "window.hpp"

constexpr uint16_t WIDTH = 800;
constexpr uint16_t HEIGHT = 800;
constexpr uint8_t SWAP_IMG_COUNT = 3;

class Viewport
{
public:
	Viewport(const Context& context, const uint16_t width = WIDTH, const uint16_t height = HEIGHT);
	~Viewport();

	//semaphore will signal when we can do something with the image
	//but can use the index for recording command purposes
	std::tuple<uint32_t, vk::Semaphore*> acquireSwapImageIndexNoFence();
	std::tuple<uint32_t, vk::Semaphore*, vk::Fence*> acquireSwapImageIndex();

	uint32_t getWidth() const;
	uint32_t getHeight() const;
	uint8_t getSwapImageCount() const;
	Swapchain& getSwapchain(); 
	vk::SwapchainKHR* getPSwapchain();
	vk::Extent2D getExtent() const;
	const std::vector<vk::ImageView>& getSwapImageViews() const;
	vk::ImageView getSwapImageView(uint32_t i) const;
	vk::PipelineViewportStateCreateInfo* getPViewportState();
	vk::Image& getSwapchainImage(uint32_t index);

private:
	const Context& context;
	XWindow window;
	Swapchain swapchain;
	uint32_t width, height;
	uint32_t swapchainImageCount;
	vk::Extent2D extent;
	vk::Viewport viewport;
	vk::Rect2D scissor;
	vk::PipelineViewportStateCreateInfo state;
};

#endif /* VIEWPORT_H */
