#ifndef COMMANDER_H
#define COMMANDER_H

#include "context.hpp"

constexpr int MAX_FRAMES_IN_FLIGHT = 8;

class Swapchain; //forward declaration

class Commander
{
public:
	Commander(const Context&);
	virtual ~Commander();

	void setSwapchain(const Swapchain&);

	void initializeCommandBuffers(const Swapchain&);

	void renderFrame(Swapchain&);

	void cleanUp();

	vk::CommandBuffer beginSingleTimeCommand();

	void endSingleTimeCommand(vk::CommandBuffer);

	void recordCopyBufferToImages(
			vk::Buffer,
			const std::vector<vk::Image>,
			uint32_t width, uint32_t height, uint32_t depth);

	void recordCopyImageToSwapImages(const Swapchain&, vk::Image);

	void copyImageToBuffer(
			vk::Image,
			vk::Buffer,
			uint32_t width, uint32_t height, uint32_t depth);

	void allocateCommandBuffersForSwapchain(const Swapchain&);

	void setSwapchainImagesToPresent(Swapchain&);

	void transitionImageLayout(
			vk::Image,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout);

	std::vector<vk::Semaphore> imageAvailableSemaphores;
	std::vector<vk::Semaphore> renderFinishedSemaphores;
	std::vector<vk::Fence> inFlightFences;

private:
	vk::CommandPool commandPool;
	std::vector<vk::CommandBuffer> commandBuffers;
	const Context& context;
	bool commandPoolCreated = false;
	bool semaphoresCreated = false;
	std::vector<vk::ClearColorValue> clearColors;
	size_t currentFrame = 0;
	size_t trueFrame = 0;
	uint32_t currentIndex = 0;

	void createCommandPool();

	void createSyncObjects();
	 
};

#endif /* COMMANDER_H */
