#ifndef COMMAND_TRANSFER_HPP
#define COMMAND_TRANSFER_HPP

#include "command.hpp"
#include <types/vktypes.hpp>

namespace sword
{

namespace render { class CommandBuffer; class Buffer; struct BufferBlock; };

namespace command
{

class CopyAttachmentToDevice : public Command
{
public:
    void execute(Application*) override;
    const char* getName() const override {return "CopyAttachmentToHost";};
    void set(std::string attachmentName, vk::Rect2D region, render::BufferBlock* block) 
    {
        this->attachmentName = attachmentName;
        this->bufferBlock = block;
        this->region = region;
    }
    void setCommandBuffer(render::CommandBuffer& cmdBuffer) {this->commandBuffer = &cmdBuffer; }
private:
    render::CommandBuffer* commandBuffer{nullptr};
    render::BufferBlock* bufferBlock{nullptr};
    std::string attachmentName;
    vk::Rect2D region;
};

}; // namespace co

}; // namespace sword

#endif /* end of include guard: COMMAND_TRANSFER_HPP */

