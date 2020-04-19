#include <state/rendermanager.hpp>

namespace sword
{

namespace state
{

RenderManager::RenderManager(StateArgs sa, Callbacks cb) :
    BranchState{sa, cb, {
        {"open_window", opcast(Op::openWindow)},
        {"prep_render_frames", opcast(Op::prepRenderFrames)},
        {"render_pass_manager", opcast(Op::renderPassMgr)},
        {"shader_manager", opcast(Op::shaderManager)},
        {"descriptor_manager", opcast(Op::descriptorManager)},
    }},
    pipelineManager{sa},
    rpassManager{sa, {[this](){activate(opcast(Op::renderPassMgr));}, nullptr}},
    descriptorManager{sa, {[this](){activate(opcast(Op::descriptorManager));}, nullptr}},
    shaderManager{sa, {[this](){activate(opcast(Op::shaderManager));}, nullptr}}
{   
    activate(opcast(Op::openWindow));
    activate(opcast(Op::shaderManager));
    activate(opcast(Op::prepRenderFrames));
    activate(opcast(Op::renderPassMgr));
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
            case Op::descriptorManager: pushState(&descriptorManager); deactivate(opcast(Op::descriptorManager)); break;
            case Op::renderPassMgr: pushState(&rpassManager); deactivate(opcast(Op::renderPassMgr)); break;
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
    activate(opcast(Op::descriptorManager));
    updateVocab();
}

}; // namespace state

}; // namespace sword


