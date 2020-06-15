#ifndef RENDER_RENDERLAYER_HPP
#define RENDER_RENDERLAYER_HPP

#include <types/vktypes.hpp>

namespace sword
{

namespace render
{

class RenderPass;
class GraphicsPipeline;
class Attachment;

// Used to be render pass instance... thinking this is 
// a better name.
class RenderLayer
{
public:
    RenderLayer(const vk::Device&, Attachment&, const RenderPass&, const GraphicsPipeline&);
    ~RenderLayer();
    RenderLayer(const RenderLayer&) = delete;
    RenderLayer& operator=(RenderLayer&) = delete;
    RenderLayer& operator=(RenderLayer&&) = delete;
    RenderLayer(RenderLayer&&);
    const RenderPass& getRenderPass() const;
    const GraphicsPipeline& getPipeline() const;
    const vk::Framebuffer& getFramebuffer() const;
private:
    vk::Framebuffer framebuffer;
    const Attachment& renderTarget;
    const RenderPass& renderPass;
    const GraphicsPipeline& pipeline;
    const vk::Device& device;
};

}; // namespace render

}; // namespace sword

#endif /* end of include guard: RENDER_RENDERLAYER_HPP */
