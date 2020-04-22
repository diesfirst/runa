#ifndef STATE_RENDERMANAGER_HPP
#define STATE_RENDERMANAGER_HPP

//imp : state/rendermanager.cpp

#include <state/state.hpp>
#include <state/pipelinemanager.hpp>
#include <state/descriptormanager.hpp>
#include <state/renderpassmanager.hpp>
#include <state/shader.hpp>
#include <command/rendercommands.hpp>

namespace sword
{

namespace state
{

class RenderManager final : public BranchState
{
public:
    const char* getName() const override { return "render_manager"; }
    void handleEvent(event::Event*) override;
    virtual ~RenderManager() = default;
    RenderManager(StateArgs, Callbacks cb);
private:
    enum class Op : Option {openWindow, prepRenderFrames, shaderManager, descriptorManager, renderPassManager, pipelineManager};

    CommandPool<command::OpenWindow> owPool;
    CommandPool<command::PrepareRenderFrames> prfPool;

    PipelineManager pipelineManager;
    RenderPassManager rpassManager;
    DescriptorManager descriptorManager;
    ShaderManager shaderManager;

    bool createdDescSetLayout{false};

    void openWindow();
    void prepRenderFrames();
    void receiveDescriptorSetLayoutReport(const DescriptorSetLayoutReport*);
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_RENDERMANAGER_HPP */
