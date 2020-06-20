#ifndef RENDER_RENDERPASS_HPP_
#define RENDER_RENDERPASS_HPP_

#include <types/vktypes.hpp>

namespace sword
{

namespace render
{

class RenderPass
{
public:
    RenderPass(const vk::Device&, const std::string name);
    ~RenderPass() = default;
    RenderPass(RenderPass&&) = default;

    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(RenderPass&) = delete;
    RenderPass& operator=(RenderPass&&) = delete;

    bool operator<(const RenderPass& rhs);
    const vk::RenderPass& getHandle() const;
    uint32_t getId() const;
    void createColorAttachment(
        const vk::Format,
        vk::ImageLayout init,
        vk::ImageLayout fin,
        vk::ClearColorValue,
        vk::AttachmentLoadOp);
    void addSubpassDependency(vk::SubpassDependency);
    void createBasicSubpassDependency();
    void createSubpass();
    void create();
    bool isCreated() const;
    bool isOffscreen() const;
    const vk::ClearValue* getClearValue() const;

private:
    const vk::Device& device;
    const std::string name;
    vk::UniqueRenderPass handle{nullptr};
    std::vector<vk::AttachmentDescription> attachments;
    std::vector<vk::AttachmentDescription> colorAttachments;
    std::vector<vk::AttachmentReference> references;
    std::vector<vk::SubpassDescription> subpasses;
    std::vector<vk::SubpassDependency> subpassDependencies;
    vk::ClearValue clearValue;
    bool created{false};
};


}; // namespace render

}; // namespace sword

#endif /* end of include guard: RENDERPASS_HPP_ */
