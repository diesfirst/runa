#ifndef RENDER_ATTACHMENT_HPP
#define RENDER_ATTACHMENT_HPP

#include <vulkan/vulkan.hpp>
#include <render/resource.hpp>

namespace sword
{

namespace render
{

class RenderPass;

class Attachment
{
public:
    Attachment(
        const vk::Device& device,
        const vk::Extent2D extent,
        const vk::ImageUsageFlags);
    Attachment(const vk::Device&, std::unique_ptr<Image>);
    Attachment(const Attachment&) = delete;
    Attachment& operator=(Attachment &&other) = default;
    Attachment& operator=(Attachment&) = delete;
    Attachment(Attachment&& other) = delete;
    virtual ~Attachment();
    vk::Framebuffer& requestFrameBuffer(const RenderPass& renderPass);
    vk::Format getFormat() const;
    vk::Extent2D getExtent() const;
    Image& getImage(uint32_t index);

private:
    friend class RenderFrame;
    const vk::Device& device;
    std::vector<std::unique_ptr<Image>> images;
    vk::Extent2D extent;
    vk::Format format;
};

}; // namespace render

}; // namespace sword

#endif /* end of include guard: RENDER_ATTACHMENT_HPP_ */
