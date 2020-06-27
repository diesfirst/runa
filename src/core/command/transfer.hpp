#ifndef COMMAND_TRANSFER_HPP
#define COMMAND_TRANSFER_HPP

#include "command.hpp"
#include <types/vktypes.hpp>

namespace sword
{

namespace render { class CommandBuffer; class Image; }

namespace command
{

class CopyAttachmentToImage : public Command
{
public:
    void execute(Application*) override;
    const char* getName() const override {return "CopyAttachmentToHost";};
    void set(std::string attachmentName, render::Image* image, vk::Rect2D region) 
    {
        this->attachmentName = attachmentName;
        this->image = image;
        this->region = region;
    }
    void setCommandBuffer(render::CommandBuffer& cmdBuffer) {this->commandBuffer = &cmdBuffer; }
private:
    render::CommandBuffer* commandBuffer{nullptr};
    render::Image* image{nullptr};
    std::string attachmentName;
    vk::Rect2D region;
};

class CopyImageToAttachment : public Command
{
public:
    void execute(Application*) override;
    const char* getName() const override {return "CopyImageToAttachment";};
    void set(render::Image* image, std::string attachmentName, vk::Rect2D region) 
    {
        this->attachmentName = attachmentName;
        this->image = image;
        this->region = region;
    }
    void setCommandBuffer(render::CommandBuffer& cmdBuffer) {this->commandBuffer = &cmdBuffer; }
private:
    render::CommandBuffer* commandBuffer{nullptr};
    render::Image* image{nullptr};
    std::string attachmentName;
    vk::Rect2D region;
};

}; // namespace co

}; // namespace sword

#endif /* end of include guard: COMMAND_TRANSFER_HPP */

