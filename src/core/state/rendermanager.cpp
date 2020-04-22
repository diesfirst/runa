#include <state/rendermanager.hpp>

namespace sword
{

namespace state
{

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
    owPool{sa.cp.openWindow},
    prfPool{sa.cp.prepareRenderFrames}
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
            case Op::openWindow: openWindow(); break;
            case Op::prepRenderFrames: prepRenderFrames(); break;
            case Op::shaderManager: pushState(&shaderManager); deactivate(opcast(Op::shaderManager)); break;
            case Op::descriptorManager: pushState(&descriptorManager); deactivate(opcast(Op::descriptorManager)); break;
            case Op::renderPassManager: pushState(&rpassManager); deactivate(opcast(Op::renderPassManager)); break;
            case Op::pipelineManager: pushState(&pipelineManager); deactivate(opcast(Op::pipelineManager)); break;
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
    rpassManager.activateCreateSwap();
    updateVocab();
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


