#include <state/rendermanager.hpp>

namespace sword
{

namespace state
{

RenderManager::RenderManager(EditStack& es, CommandStack& cs, ExitCallbackFn cb) :
    BranchState{es, cs, cb, {
        {"open_window", opcast(Op::openWindow)},
        {"prep_render_frames", opcast(Op::prepRenderFrames)},
        {"shader_manager", opcast(Op::shaderManager)}
    }},
    pipelineManager{es, cs},
    rpassManager{es, cs},
    descriptorManager{es, cs},
    shaderManager{es, cs, [this](){activate(opcast(Op::shaderManager));}}
{   
    activate(opcast(Op::openWindow));
    activate(opcast(Op::shaderManager));
    activate(opcast(Op::prepRenderFrames));
}
void RenderManager::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        Optional option = extractCommand(event);
        if (!option) return;
        switch(opcast(*option))
        {
            case Op::openWindow: openWindow(); break;
            case Op::prepRenderFrames: prepRenderFrames(); break;
            case Op::shaderManager: pushState(&shaderManager); deactivate(opcast(Op::shaderManager)); break;
        }
    }
}

void RenderManager::openWindow()
{
    auto cmd = owPool.request();
    pushCmd(std::move(cmd));
    deactivate(opcast(Op::openWindow));
    updateVocab();
}

void RenderManager::prepRenderFrames()
{
    auto cmd = prfPool.request();
    pushCmd(std::move(cmd));
    deactivate(opcast(Op::prepRenderFrames));
    updateVocab();
}

}; // namespace state

}; // namespace sword


