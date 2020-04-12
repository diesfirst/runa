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
    enum class Op : Option {openWindow, prepRenderFrames, shaderManager};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    const char* getName() const override { return "render_manager"; }
    void handleEvent(event::Event*) override;
    virtual ~RenderManager() = default;
    RenderManager(EditStack& es, CommandStack& cs)  : 
        BranchState{es, cs,{
            {"open_window", opcast(Op::openWindow)},
            {"prep_render_frames", opcast(Op::prepRenderFrames)},
            {"shader_manager", opcast(Op::shaderManager)}
        }},
        pipelineManager{es, cs},
        rpassManager{es, cs},
        descriptorManager{es, cs},
        shaderManager{es, cs}
    {   
        activate(opcast(Op::openWindow));
        activate(opcast(Op::shaderManager));
        activate(opcast(Op::prepRenderFrames));
    }
private:
    CommandPool<command::OpenWindow> owPool;
    CommandPool<command::PrepareRenderFrames> prfPool;

    PipelineManager pipelineManager;
    RenderpassManager rpassManager;
    DescriptorManager descriptorManager;
    ShaderManager shaderManager;

    void openWindow();
    void prepRenderFrames();
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_RENDERMANAGER_HPP */
