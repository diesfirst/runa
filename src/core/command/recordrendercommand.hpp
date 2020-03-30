#ifndef RECORDRENDERCOMMAND_HPP_
#define RECORDRENDERCOMMAND_HPP_

#include <command/command.hpp>
#include <state/state.hpp>

namespace sword
{

namespace command
{

class RecordRenderCommand : public Command
{
public:
    void execute(Application*) override {}
    const char* getName() const override { return "foo";}
    inline void set(int index, std::vector<uint32_t> renderpassInstances)
    {
        this->cmdBufferId = index;
        this->renderpassInstances = renderpassInstances;
    }
private:
    int cmdBufferId;
    std::vector<uint32_t> renderpassInstances;
};

}; //command

namespace state
{

class RendererManager;

class RecordRenderCommandState : public LeafState
{
public:
    RecordRenderCommandState(EditStack& es, CommandStack& cs, RendererManager& parent) :
       state::LeafState{es, cs}, parent{parent} {}
    const char* getName() const override {return cmd.getName();}
    void onEnterExt(Application*) override {std::cout << "Enter a command buffer id and the render pass instances." << std::endl;}
private:
    command::RecordRenderCommand cmd;
    RendererManager& parent;
};

}; //state

}; //sword

#endif /* end of include guard: RECORDRENDERCOMMAND_HPP_ */
