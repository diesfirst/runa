#include "command.hpp"

namespace sword
{

namespace render { class CommandBuffer; };

namespace command
{

class CopyAttachmentToDevice : public Command
{
public:
    void execute(Application*) override;
    const char* getName() const override {return "CopyAttachmentToHost";};
    void set() {}
    void setCommandBuffer(render::CommandBuffer&);
private:
    
};

}; // namespace co

}; // namespace sword
