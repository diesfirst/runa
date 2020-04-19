#ifndef STATE_RENDERMANAGER_HPP
#define STATE_RENDERMANAGER_HPP

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
    enum class Op : Option {renderPassMgr, openWindow, prepRenderFrames, shaderManager, descriptorManager};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    const char* getName() const override { return "render_manager"; }
    void handleEvent(event::Event*) override;
    virtual ~RenderManager() = default;
    RenderManager(StateArgs, Callbacks cb);
private:
    CommandPool<command::OpenWindow> owPool;
    CommandPool<command::PrepareRenderFrames> prfPool;

    PipelineManager pipelineManager;
    RenderPassManager rpassManager;
    DescriptorManager descriptorManager;
    ShaderManager shaderManager;

    void openWindow();
    void prepRenderFrames();
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_RENDERMANAGER_HPP */
