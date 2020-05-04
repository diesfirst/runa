#ifndef RENDER_RENDERFRAME_HPP
#define RENDER_RENDERFRAME_HPP

#include <vulkan/vulkan.hpp>
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
    RenderFrame(const RenderFrame&) = delete;
    RenderFrame& operator=(RenderFrame&) = delete;
    RenderFrame& operator=(RenderFrame&& other) = delete;
    RenderFrame(RenderFrame&& other);
    virtual ~RenderFrame();

    Attachment& getSwapAttachment();
    void addOffscreenAttachment(std::unique_ptr<Attachment>&& renderTarget);
    void createDescriptorSets(const std::vector<vk::DescriptorSetLayout>&);
    const std::vector<vk::DescriptorSet>& getDescriptorSets() const;
    vk::Semaphore requestSemaphore();
    CommandBuffer& requestRenderBuffer(uint32_t bufferId); //will reset if exists
    CommandBuffer& getRenderBuffer(uint32_t bufferId);  //will fetch existing
    RenderLayer& getRenderLayer(int id) { return renderLayers.at(id);}
    void addRenderLayer(Attachment&, const RenderPass&, const GraphicsPipeline&);
    void addRenderLayer(const RenderPass&, const GraphicsPipeline&);
    void clearRenderPassInstances();
    BufferBlock* bufferBlock;
    std::vector<CommandBuffer*> commandBuffers;
    

private:
    std::unique_ptr<Attachment> swapchainAttachment;
    std::vector<RenderLayer> renderLayers;
    const Context& context;
    const vk::Device& device;
    CommandPool commandPool;
//	CommandBuffer* renderBuffer{nullptr};
    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;
    void updateDescriptorSet(uint32_t setId, const std::vector<vk::WriteDescriptorSet>);
    vk::Fence fence;
    vk::Semaphore semaphore;
    uint32_t width;
    uint32_t height;

    void createDescriptorPool();
};

}; // namespace render

}; // namespace sword

#endif /* end of include guard: RENDER_RENDERFRAME_HPP */
