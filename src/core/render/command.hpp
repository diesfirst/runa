#ifndef RENDER_COMMAND_HPP
#define RENDER_COMMAND_HPP

#include <types/vktypes.hpp>

namespace sword
{

namespace render
{

class CommandBuffer; //forward declaratiohn

class CommandPool
{
friend class CommandBuffer;
public:
    CommandPool(
        const vk::Device&, 
        const vk::Queue,
        uint32_t queueFamilyIndex, 
        vk::CommandPoolCreateFlags = {}); //empty flags by default
    ~CommandPool();
    CommandPool(CommandPool&&);
    CommandPool(const CommandPool&) = delete;
    CommandPool& operator=(CommandPool&&) = delete;
    CommandPool& operator=(const CommandPool&) = delete;
    CommandBuffer& requestCommandBuffer(
    vk::CommandBufferLevel = vk::CommandBufferLevel::ePrimary);
    void resetPool();

private:
    const vk::Device& device;
    const vk::Queue queue{nullptr};
    vk::CommandPool handle{nullptr};
    std::vector<std::unique_ptr<CommandBuffer>> primaryCommandBuffers;
    uint32_t activePrimaryCommandBufferCount{0};
};

class CommandBuffer
{
public: 
    CommandBuffer(
        CommandPool& pool, 
        vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
    ~CommandBuffer();
    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(CommandBuffer&) = delete;
    CommandBuffer& operator=(CommandBuffer&&) = delete;
    CommandBuffer(CommandBuffer&&);
    
    void begin();
    void beginRenderPass(vk::RenderPassBeginInfo&);
    void bindGraphicsPipeline(const vk::Pipeline& pipeline);
    void bindDescriptorSets(
    const vk::PipelineLayout& layout, 
    const std::vector<vk::DescriptorSet>& sets, 
    const std::vector<uint32_t>& offsets);
    void drawVerts(uint32_t vertCount, uint32_t firstVertex);
    void insertImageMemoryBarrier(
    vk::PipelineStageFlags srcStageMask,
    vk::PipelineStageFlags dstStageMask,
    vk::ImageMemoryBarrier imb);
    void copyImageToBuffer(vk::Image&, vk::Buffer&, vk::BufferImageCopy);
    void endRenderPass();
    void end();
    vk::Semaphore submit(vk::Semaphore& waitSemaphore, vk::PipelineStageFlags);
    void submit();
    bool isRecorded() const;
    void waitForFence() const;
    void reset();
private:
    const vk::Device& device;
    const vk::Queue& queue{nullptr};
    std::vector<vk::CommandBuffer> buffers;
    vk::CommandBuffer handle{nullptr};
    vk::Semaphore signalSemaphore{nullptr};
    vk::Fence fence{nullptr};
    bool recordingComplete{false};
};

}; // namespace render

}; // namespace sword
	 
#endif /* end of include guard: RENDER_COMMAND_HPP */


