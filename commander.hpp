#ifndef COMMANDER_H
#define COMMANDER_H

#include "context.hpp"
#include "swapchain.hpp"

constexpr int MAX_FRAMES_IN_FLIGHT = 8;

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

	void recordCopyBufferToSwapImages(const Swapchain&, vk::Buffer);

	void allocateCommandBuffersForSwapchain(const Swapchain&);

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
	uint32_t currentIndex = 0;

	void createCommandPool();

	void createClearColors();

	void createSemaphores();
	 
};

#endif /* COMMANDER_H */
