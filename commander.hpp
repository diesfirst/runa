#ifndef COMMANDER_H
#define COMMANDER_H

#include "context.hpp"
#include "swapchain.hpp"
#include "painter.hpp"


class Commander
{
public:
	Commander(const Context&);
	virtual ~Commander();

	void setSwapchain(const Swapchain&);

	void initializeCommandBuffers(const Swapchain&, const Painter&);

	void renderFrame(Swapchain&);

	void cleanUp();

	void singleCommand();

	vk::Semaphore imageAvailableSemaphore;
	vk::Semaphore renderFinishedSemaphore;

private:
	vk::CommandPool commandPool;
	std::vector<vk::CommandBuffer> commandBuffers;
	const Context& context;
	bool commandPoolCreated = false;
	bool semaphoresCreated = false;
	std::vector<vk::ClearColorValue> clearColors;

	void createCommandPool();

	void createClearColors();

	void recordCommandBuffers(const Swapchain&, const Painter&);

	void allocateCommandBuffers(const Swapchain&);

	void createSemaphores();
	 
};

#endif /* COMMANDER_H */
