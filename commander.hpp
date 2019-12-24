#ifndef COMMANDER_H
#define COMMANDER_H

#include <vulkan/vulkan.hpp>

struct DrawableInfo
{
	uint32_t indexBufferOffset;
	uint32_t numberOfIndices;
	uint32_t vertexIndexOffset;
};

class CommandBuffer; //forward declaratiohn

class CommandPool
{
friend class CommandBuffer;
public:
	CommandPool(
			const vk::Device&, 
			const vk::Queue&,
			uint32_t queueFamilyIndex, 
			vk::CommandPoolCreateFlags = {}); //empty flags by default
	virtual ~CommandPool();
	CommandPool(CommandPool&&);
	CommandPool(const CommandPool&) = delete;
	CommandPool& operator=(CommandPool&&) = delete;
	CommandPool& operator=(const CommandPool&) = delete;
	CommandBuffer& requestCommandBuffer(
			vk::CommandBufferLevel = vk::CommandBufferLevel::ePrimary);
	void resetPool();

private:
	const vk::Device& device;
	const vk::Queue& queue;
	vk::CommandPool handle;
	std::vector<std::unique_ptr<CommandBuffer>> primaryCommandBuffers;
	uint32_t activePrimaryCommandBufferCount;
};


class CommandBuffer
{
public: 
	CommandBuffer(
			CommandPool& pool, 
			vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
	virtual ~CommandBuffer();
	CommandBuffer(const CommandBuffer&) = delete;
	CommandBuffer& operator=(CommandBuffer&) = delete;
	CommandBuffer& operator=(CommandBuffer&&) = delete;
	CommandBuffer(CommandBuffer&&);
	
	void begin();
	void beginRenderPass(vk::RenderPassBeginInfo&);
	void bindGraphicsPipeline(vk::Pipeline& pipeline);
	void drawVerts(uint32_t vertCount, uint32_t firstVertex);
	void endRenderPass();
	void end();
	vk::Semaphore submit(vk::Semaphore& waitSemaphore, vk::PipelineStageFlags);
	const bool isRecorded() const;
private:
	CommandPool& pool;
	const vk::Device& device;
	const vk::Queue& queue;
	std::vector<vk::CommandBuffer> buffers;
	vk::CommandBuffer handle;
	vk::Semaphore signalSemaphore;
	vk::Fence fence;
	bool recordingComplete{false};
};

class Commander
{
public:
	Commander(vk::Device& device, vk::Queue& queue, uint32_t queueFamily);
	virtual ~Commander();

	void createCommandPool(uint32_t queueFamily); // must be called

	CommandBuffer requestCommandBuffer() const;
	
	void submitDrawCommand(
		CommandBuffer* drawCommand, 
		vk::Semaphore* pImageAcquiredSemaphore, 
		uint32_t swapImageIndex);

	uint8_t renderFrame(const vk::SwapchainKHR* swapchain);
	
	void presentSwapImage(vk::SwapchainKHR& swapchain, uint32_t index);

	void cleanUp();

	vk::CommandBuffer beginSingleTimeCommand();

	void endSingleTimeCommand(vk::CommandBuffer);

	void recordCopyBufferToImages(
			std::vector<vk::CommandBuffer>&,
			vk::Buffer&,
			const std::vector<vk::Image>&,
			uint32_t width, uint32_t height);
	
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
		vk::Buffer& buffer,
		vk::Image& image,
		uint32_t width, uint32_t height,
		uint32_t bufferOffset);

	void copyImageToBuffer(
			vk::Image&,
			vk::Buffer&,
			uint32_t width, uint32_t height, uint32_t depth);

	void copyImageToImage(
			vk::Image&,
			vk::Image&,
			uint32_t width, uint32_t height);

	void allocateDrawCommandBuffers(const uint32_t count);

	void transitionImageLayout(
			vk::Image&,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout);

	void resetCommandBuffers();
	void resetCommandBuffer(uint32_t index);
	void resetCurrentBuffer();

	std::vector<vk::Semaphore> imageAvailableSemaphores;
	std::vector<vk::Semaphore> renderFinishedSemaphores;
	std::vector<vk::Fence> inFlightFences;

private:
	std::unique_ptr<CommandPool> commandPool;
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
