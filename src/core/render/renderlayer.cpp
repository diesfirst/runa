#include <render/renderlayer.hpp>
#include <render/attachment.hpp>
#include <render/renderpass.hpp>

namespace sword
{

namespace render
{

RenderLayer::RenderLayer(
        const vk::Device& device,
        Attachment& target, 
        const RenderPass& pass,
        const GraphicsPipeline& pipe, 
        const DrawParms drawParms) :
    renderTarget{target},
    renderPass{pass},
    pipeline{pipe},
    device{device},
    drawParms{drawParms}
{
    vk::FramebufferCreateInfo ci;
    ci.setWidth(target.getExtent().width);
    ci.setHeight(target.getExtent().height);
    ci.setAttachmentCount(1);
    ci.setPAttachments(&target.getImage(0).getView());
    ci.setLayers(1);
    ci.setRenderPass(pass.getHandle()); //only a template for where this fb can be used
    framebuffer = device.createFramebufferUnique(ci);
}

const RenderPass& RenderLayer::getRenderPass() const
{
    return renderPass;
}

const GraphicsPipeline& RenderLayer::getPipeline() const
{
    return pipeline;
}

const vk::Framebuffer& RenderLayer::getFramebuffer() const
{
    return *framebuffer;
}

const DrawParms RenderLayer::getDrawParms() const
{
    return drawParms;
}

}; // namespace render

}; // namespace sword
