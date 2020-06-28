#ifndef RENDER_RENDERFRAME_HPP
#define RENDER_RENDERFRAME_HPP

#include <types/vktypes.hpp>
#include <render/command.hpp>
#include <render/renderlayer.hpp>

namespace sword
{

namespace render
{

class Context;
class Attachment;
class CommandBuffer;
class GraphicsPipeline;
class RenderPass;
class BufferBlock;

class RenderFrame
{
public:
    RenderFrame(
        const Context&, 
        std::unique_ptr<Attachment>&& renderTarget, 
        uint32_t width, uint32_t height);
    ~RenderFrame() = default;
    RenderFrame(RenderFrame&& other) = default;

    RenderFrame(const RenderFrame&) = delete;
    RenderFrame& operator=(RenderFrame&) = delete;
    RenderFrame& operator=(RenderFrame&& other) = delete;

    Attachment& getSwapAttachment();
    void addOffscreenAttachment(std::unique_ptr<Attachment>&& renderTarget);
    void createDescriptorSets(const std::vector<vk::DescriptorSetLayout>&);
    const std::vector<vk::UniqueDescriptorSet>& getDescriptorSets() const;
    vk::Semaphore requestSemaphore();
    CommandBuffer& requestRenderBuffer(uint32_t bufferId); //will reset if exists
    CommandBuffer& getRenderBuffer(uint32_t bufferId);  //will fetch existing
    RenderLayer& getRenderLayer(int id) { return renderLayers.at(id);}
    void addRenderLayer(RenderLayer&&);
    void addRenderLayer(const RenderPass&, const GraphicsPipeline&, const vk::Device& device);
    void clearRenderPassInstances();
    void addUniformBufferBlock(BufferBlock*);
    BufferBlock* getUniformBufferBlock(int index);

private:
    std::unique_ptr<Attachment> swapchainAttachment;
    std::vector<RenderLayer> renderLayers;
    CommandPool commandPool;
    vk::UniqueDescriptorPool descriptorPool;
    vk::UniqueFence fence;
    vk::UniqueSemaphore semaphore;
    std::vector<vk::UniqueDescriptorSet> descriptorSets;
    std::vector<BufferBlock*> uniformBufferBlocks;
    void updateDescriptorSet(uint32_t setId, const std::vector<vk::WriteDescriptorSet>);
    uint32_t width;
    uint32_t height;

    void createDescriptorPool();
};

}; // namespace render

}; // namespace sword

#endif /* end of include guard: RENDER_RENDERFRAME_HPP */
