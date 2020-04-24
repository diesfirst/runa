#include <state/rendermanager.hpp>

namespace sword
{

namespace state
{

RecordRenderCommand::RecordRenderCommand(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}
{}

void RecordRenderCommand::onEnterExt()
{
    std::cout << "Enter a command index, and an array of render layer ids to use." << std::endl;
}

void RecordRenderCommand::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);       
    }
}

CreateRenderLayer::CreateRenderLayer(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}, crlPool{sa.cp.createRenderLayer}
{
    sa.rg.createRenderLayer = this;
}

void CreateRenderLayer::onEnterExt()
{
    std::cout << "Enter an attachment name (or swap), a render pass name, and a pipeline name" << std::endl;
}

void CreateRenderLayer::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto attachmentName = ce->getArg<std::string, 0>();
        auto renderPassName = ce->getArg<std::string, 1>();
        auto pipelineName = ce->getArg<std::string, 2>();
        auto cmd = crlPool.request(reportCallback(), attachmentName, renderPassName, pipelineName);
        pushCmd(std::move(cmd));
        popSelf();
        event->setHandled();
    }
}

OpenWindow::OpenWindow(StateArgs sa, Callbacks cb) :
    BriefState{sa, cb}, owPool{sa.cp.openWindow}
{
    sa.rg.openWindow = this;
}

void OpenWindow::onEnterExt()
{
    auto cmd = owPool.request(reportCallback());
    pushCmd(std::move(cmd));
    popSelf();
}

PrepareRenderFrames::PrepareRenderFrames(StateArgs sa, Callbacks cb) :
    BriefState{sa, cb}, prfPool{sa.cp.prepareRenderFrames}
{
    sa.rg.prepareRenderFrames = this;
}

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
        {"pipeline_manager", opcast(Op::pipelineManager)},
        {"create_render_layer", opcast(Op::createRenderLayer)},
        {"record_render_command", opcast(Op::recordRenderCommand)},
        {"print_reports", opcast(Op::printReports)}
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
    openWindow{sa, {nullptr, [this](Report* report) { deactivate(opcast(Op::openWindow)); }}},
    prepRenderFrames{sa, {nullptr, [this](Report* report) { onPrepRenderFrames(); }}},
    createRenderLayer{sa, {nullptr, [this](Report* report) { addReport(report, &renderLayersReports); }}},
    recordRenderCommand{sa, {nullptr, [this](Report* report) { addReport(report, &renderCommandReports); }}}
{   
    activate(opcast(Op::openWindow));
    activate(opcast(Op::shaderManager));
    activate(opcast(Op::prepRenderFrames));
    activate(opcast(Op::renderPassManager));
    activate(opcast(Op::printReports));
    activate(opcast(Op::createRenderLayer));
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
            case Op::createRenderLayer: pushState(&createRenderLayer); break;
            case Op::recordRenderCommand: pushState(&recordRenderCommand); break;
            case Op::printReports: printReports(); break;
        }
    }
}

void RenderManager::onPrepRenderFrames()
{
    deactivate(opcast(Op::prepRenderFrames));
    activate(opcast(Op::descriptorManager));
    rpassManager.activateCreateSwap();
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

void RenderManager::printReports() const
{
    for (const auto& r : renderLayersReports) 
    {
        std::invoke(*r);
    }
    for (const auto& r : renderCommandReports) 
    {
        std::invoke(*r);
    }
}

}; // namespace state

}; // namespace sword


