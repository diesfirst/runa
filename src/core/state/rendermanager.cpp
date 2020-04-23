#include <state/rendermanager.hpp>

namespace sword
{

namespace state
{

OpenWindow::OpenWindow(StateArgs sa, Callbacks cb) :
    BriefState{sa, cb}, owPool{sa.cp.openWindow}
{}

void OpenWindow::onEnterExt()
{
    auto cmd = owPool.request(reportCallback());
    pushCmd(std::move(cmd));
    popSelf();
}

PrepareRenderFrames::PrepareRenderFrames(StateArgs sa, Callbacks cb) :
    BriefState{sa, cb}, prfPool{sa.cp.prepareRenderFrames}
{}

void PrepareRenderFrames::onEnterExt()
{
    auto cmd = prfPool.request(reportCallback());
    pushCmd(std::move(cmd));
    popSelf();
}

RenderManager::RenderManager(StateArgs sa, Callbacks cb) :
    BranchState{sa, cb, {
        {"descriptor_manager", opcast(Op::descriptorManager)},
        {"prep_render_frames", opcast(Op::prepRenderFrames)},
        {"render_pass_manager", opcast(Op::renderPassManager)},
        {"open_window", opcast(Op::openWindow)},
        {"shader_manager", opcast(Op::shaderManager)},
        {"pipeline_manager", opcast(Op::pipelineManager)}
    }},
    pipelineManager{sa, {[this](){activate(opcast(Op::pipelineManager));}}},
    rpassManager{sa, {
        [this](){activate(opcast(Op::renderPassManager));}},
        [this](const RenderPassReport* r){ pipelineManager.receiveReport(r); }},
    descriptorManager{sa, {
        [this](){activate(opcast(Op::descriptorManager));}}, 
        [this](const DescriptorSetLayoutReport* r){ receiveDescriptorSetLayoutReport(r); }},
    shaderManager{sa, {
        [this](){activate(opcast(Op::shaderManager));}},
        [this](const ShaderReport* r){ pipelineManager.receiveReport(r); }},
    openWindow{sa, {nullptr, [this](Report* report) { onOpenWindow(); }}},
    prepRenderFrames{sa, {nullptr, [this](Report* report) { onPrepRenderFrames(); }}}
{   
    activate(opcast(Op::openWindow));
    activate(opcast(Op::shaderManager));
    activate(opcast(Op::prepRenderFrames));
    activate(opcast(Op::renderPassManager));
}
void RenderManager::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        Optional option = extractCommand(event);
        if (!option) return;
        switch(opcast<Op>(*option))
        {
            case Op::openWindow: pushState(&openWindow); break;
            case Op::prepRenderFrames: pushState(&prepRenderFrames); break;
            case Op::shaderManager: pushState(&shaderManager); deactivate(opcast(Op::shaderManager)); break;
            case Op::descriptorManager: pushState(&descriptorManager); deactivate(opcast(Op::descriptorManager)); break;
            case Op::renderPassManager: pushState(&rpassManager); deactivate(opcast(Op::renderPassManager)); break;
            case Op::pipelineManager: pushState(&pipelineManager); deactivate(opcast(Op::pipelineManager)); break;
        }
    }
}

void RenderManager::onPrepRenderFrames()
{
    deactivate(opcast(Op::prepRenderFrames));
    activate(opcast(Op::descriptorManager));
    rpassManager.activateCreateSwap();
}

void RenderManager::onOpenWindow()
{
    deactivate(opcast(Op::openWindow));
}

void RenderManager::receiveDescriptorSetLayoutReport(const DescriptorSetLayoutReport* r)
{
    if (!createdDescSetLayout)
    {
        activate(opcast(Op::pipelineManager));
        createdDescSetLayout = true;
    }
    pipelineManager.receiveReport(r); 
}

}; // namespace state

}; // namespace sword


