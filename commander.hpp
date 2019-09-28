#ifndef COMMANDER_H
#define COMMANDER_H

#include <vulkan/vulkan.hpp>

constexpr int MAX_FRAMES_IN_FLIGHT = 3;

class Swapchain; //forward declaration

class Commander
{
public:
	Commander(vk::Device& device, vk::Queue& queue, uint32_t queueFamily);
	virtual ~Commander();

	void createCommandPool(uint32_t queueFamily); // must be called

	void createSyncObjects();

	void setQueue(vk::Queue& queue); 

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

	
	void recordDrawVert(
		vk::RenderPass& renderpass,
	 	std::vector<vk::Framebuffer>& framebuffers,
		vk::Buffer& vertexBuffer,
		vk::Pipeline& graphicsPipeline,
		vk::PipelineLayout& pipelineLayout,
		uint32_t width, uint32_t height,
		uint32_t vertexCount);
		
	void recordCopyImageToSwapImages(const Swapchain&, vk::Image);

	void copyImageToBuffer(
			vk::Image,
			vk::Buffer,
			uint32_t width, uint32_t height, uint32_t depth);

	void allocateCommandBuffers(const uint32_t count);

	void setSwapchainImagesToPresent(Swapchain&);

	void transitionImageLayout(
			vk::Image,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout);

	void resetCommandBuffers();
	void resetCommandBuffer(uint32_t index);
	void resetCurrentBuffer();

	std::vector<vk::Semaphore> imageAvailableSemaphores;
	std::vector<vk::Semaphore> renderFinishedSemaphores;
	std::vector<vk::Fence> inFlightFences;

private:
	vk::CommandPool commandPool;
	std::vector<vk::CommandBuffer> commandBuffers;
	const vk::Device& device;
	vk::Queue& queue;
	bool commandPoolCreated = false;
	bool semaphoresCreated = false;
	std::vector<vk::ClearColorValue> clearColors;
	size_t currentFrame = 0;
	size_t trueFrame = 0;
	uint32_t currentIndex = 0;

	 
};

#endif /* COMMANDER_H */
