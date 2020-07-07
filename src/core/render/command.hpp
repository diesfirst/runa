#ifndef RENDER_COMMAND_HPP
#define RENDER_COMMAND_HPP

#include <types/vktypes.hpp>

namespace sword
{

namespace render
{


class CommandBuffer; //forward declaratiohn

void waitOnCommandBuffer(CommandBuffer&, const vk::Device&);

template <size_t Size>
class CommandPool_t
{
friend class CommandBuffer;
public:
    CommandPool_t(
        const vk::Device&, 
        const vk::Queue,
        uint32_t queueFamilyIndex, 
        vk::CommandPoolCreateFlags = {}); //empty flags by default
    ~CommandPool_t() = default;
    CommandPool_t(CommandPool_t&&) = default;

    CommandPool_t(const CommandPool_t&) = delete;
    CommandPool_t& operator=(CommandPool_t&&) = delete;
    CommandPool_t& operator=(const CommandPool_t&) = delete;

    CommandBuffer& requestCommandBuffer() { return *primaryCommandBuffers.at(0); };
    CommandBuffer& requestCommandBuffer(uint32_t id, 
    vk::CommandBufferLevel = vk::CommandBufferLevel::ePrimary);
    void resetPool();

private:
    const vk::Queue queue{nullptr};
    vk::UniqueCommandPool handle{nullptr};
    std::array<std::unique_ptr<CommandBuffer>, Size> primaryCommandBuffers;
    uint32_t activePrimaryCommandBufferCount{0};
};

using CommandPool = CommandPool_t<5>;

class CommandBuffer
{
public: 
    CommandBuffer(
        vk::UniqueCommandBuffer&& buffer,
        const vk::Device& device,
        const vk::Queue,
        vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
    ~CommandBuffer() = default;
    CommandBuffer(CommandBuffer&&) = default;

    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(CommandBuffer&) = delete;
    CommandBuffer& operator=(CommandBuffer&&) = delete;
    
    void begin();
    void beginRenderPass(vk::RenderPassBeginInfo&);
    void bindGraphicsPipeline(const vk::Pipeline& pipeline);
    void bindDescriptorSets(
        const vk::PipelineLayout& layout, 
        const std::vector<vk::DescriptorSet>& sets, 
        const std::vector<uint32_t>& offsets);
    void bindVertexBuffer(uint32_t firstBinding, const vk::Buffer*, uint32_t offset);
    void drawVerts(uint32_t vertCount, uint32_t firstVertex);
    void insertImageMemoryBarrier(
    vk::PipelineStageFlags srcStageMask,
    vk::PipelineStageFlags dstStageMask,
    vk::ImageMemoryBarrier imb);
    void copyImageToBuffer(vk::Image&, vk::Buffer&, vk::BufferImageCopy);
    void copyImageToImage(vk::Image&, vk::Image&, vk::ImageCopy);
    void endRenderPass();
    void end();
    vk::Semaphore submit(vk::Semaphore& waitSemaphore, vk::PipelineStageFlags);
    void submit();
    bool isRecorded() const;
    void waitForFence() const;
    const vk::Fence& getFence();
    void reset();

private:
    const vk::Queue queue{nullptr};
    vk::UniqueCommandBuffer handle{nullptr};
    vk::UniqueSemaphore signalSemaphore{nullptr};
    vk::UniqueFence fence{nullptr};
    bool recordingComplete{false};
};

}; // namespace render

}; // namespace sword
	 
#endif /* end of include guard: RENDER_COMMAND_HPP */


