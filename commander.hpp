#ifndef COMMANDER_H
#define COMMANDER_H

#include <vulkan/vulkan.hpp>

constexpr int MAX_FRAMES_IN_FLIGHT = 3;

struct DrawableInfo
{
	uint32_t indexBufferOffset;
	uint32_t numberOfIndices;
	uint32_t vertexIndexOffset;
};


class CommandBlock
{
public: 
	CommandBlock(const vk::CommandPool& pool, const vk::Device& device, uint32_t size);
	~CommandBlock();
	std::vector<vk::CommandBuffer>& getCommandBuffers();
	uint32_t getSize() const;
	vk::CommandBuffer* getPCommandBuffer(uint32_t index);
	std::vector<vk::CommandBuffer> commandBuffers;
	std::vector<vk::Semaphore> semaphores;
	std::vector<vk::Fence> fences;
private:
	uint32_t size;
	const vk::Device& device;
	void initSemaphores(uint32_t size);
	void initFences(uint32_t size);
};

class Commander
{
public:
	Commander(vk::Device& device, vk::Queue& queue, uint32_t queueFamily);
	virtual ~Commander();

	void createCommandPool(uint32_t queueFamily); // must be called

	std::unique_ptr<CommandBlock> createCommandBlock(const uint32_t count) const;
	
	void submitDrawCommand(
		CommandBlock* drawCommandBlock, 
		vk::Semaphore* pImageAcquiredSemaphore, 
		uint32_t swapImageIndex);

	uint8_t renderFrame(const vk::SwapchainKHR* swapchain);
	
	void presentSwapImage(vk::SwapchainKHR& swapchain, uint32_t index);

	void cleanUp();

	vk::CommandBuffer beginSingleTimeCommand();

	void endSingleTimeCommand(vk::CommandBuffer);

	void recordCopyBufferToImages(
			std::vector<vk::CommandBuffer>&,
			vk::Buffer,
			const std::vector<vk::Image>,
			uint32_t width, uint32_t height);

	
	void recordDraw(
		vk::RenderPass& renderpass,
	 	std::vector<vk::Framebuffer>& framebuffers,
		vk::Buffer& vertexBuffer,
		vk::Pipeline& graphicsPipeline,
		vk::PipelineLayout& pipelineLayout,
		uint32_t width, uint32_t height,
		uint32_t vertexCount);

	void recordDraw(
		std::vector<vk::CommandBuffer>& commandBuffers,
		vk::RenderPass& renderpass,
	 	std::vector<vk::Framebuffer>& framebuffers,
		vk::Buffer& vertexBuffer,
		vk::Buffer& indexBuffer,
		vk::Pipeline& graphicsPipeline,
		vk::PipelineLayout& pipelineLayout,
		std::vector<vk::DescriptorSet>& descriptorSets,
		uint32_t width, uint32_t height,
		std::vector<DrawableInfo>&, uint32_t dynamicAlignment);
		
	void copyBufferToImageSingleTime(
		vk::Buffer buffer,
		vk::Image image,
		uint32_t width, uint32_t height,
		uint32_t bufferOffset);

	void copyImageToBuffer(
			vk::Image,
			vk::Buffer,
			uint32_t width, uint32_t height, uint32_t depth);

	void allocateDrawCommandBuffers(const uint32_t count);

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
	std::vector<vk::CommandBuffer> drawCommandBuffers;
	const vk::Device& device;
	vk::Queue& graphicsQueue;
	vk::Queue& primaryQueue;
	bool commandPoolCreated = false;
	std::vector<vk::ClearColorValue> clearColors;
	uint8_t currentFrame = 0;
	size_t trueFrame = 0;
	uint32_t currentIndex = 0;

	 
};

#endif /* COMMANDER_H */
