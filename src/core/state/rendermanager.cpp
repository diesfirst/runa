#include <state/rendermanager.hpp>

namespace sword
{

namespace state
{

SaveSwapImage::SaveSwapImage(StateArgs sa, Callbacks cb) : 
    LeafState{sa, cb}, pool{sa.cp.saveSwapToPng}
{}

void SaveSwapImage::onEnterExt()
{
    std::cout << "Enter a name for the file." << '\n';
}

void SaveSwapImage::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto name = ce->getArg<std::string, 0>();
        if (name == "")
        {
            std::cout << "Name cannot be empty." << '\n';
            event->setHandled();
            popSelf();
            return;
        }
        auto cmd = pool.request(reportCallback(), name);
        pushCmd(std::move(cmd));
        event->setHandled();
        popSelf();
    }
}

SetRenderCommand::SetRenderCommand(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}
{}

void SetRenderCommand::onEnterExt()
{
    std::cout << "Enter a render command index" << '\n';
}

void SetRenderCommand::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto commandIndex = ce->getArg<int, 0>();
        auto cmd = pool.request(reportCallback(), commandIndex);
        pushCmd(std::move(cmd));
        popSelf();
        event->setHandled();
    }
}

RecordRenderCommand::RecordRenderCommand(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}, pool{sa.cp.recordRenderCommand}
{
    sa.rg.recordRenderCommand = this;
}

void RecordRenderCommand::onEnterExt()
{
    std::cout << "Enter a command index, and an array of render layer ids to use." << std::endl;
}

void RecordRenderCommand::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);       
        auto ss = ce->getStream();
        int cmdBufferId;
        uint32_t holder;
        std::vector<uint32_t> renderLayerIds;
        ss >> cmdBufferId;
        while (ss >> holder)
            renderLayerIds.push_back(holder);
        auto cmd = pool.request(reportCallback(), cmdBufferId, renderLayerIds);
        pushCmd(std::move(cmd));
        popSelf();
        event->setHandled();
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
        {"render", opcast(Op::render)},
        {"print_reports", opcast(Op::printReports)},
        {"save_swap_image", opcast(Op::saveSwapImage)}
    }},
    pipelineManager{sa, {
        [this](){activate(opcast(Op::pipelineManager));}},
        [this](const GraphicsPipelineReport* r) { receiveGraphicsPipelineReport(r); }},
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
    recordRenderCommand{sa, {nullptr, [this](Report* report) { addReport(report, &renderCommandReports); }}},
    setRenderCommand{sa, {}},
    saveSwapImage{sa, {}},
    rrcPool{sa.cp.recordRenderCommand}
{   
    activate(opcast(Op::openWindow));
    activate(opcast(Op::shaderManager));
    activate(opcast(Op::prepRenderFrames));
    activate(opcast(Op::renderPassManager));
    activate(opcast(Op::printReports));
    activate(opcast(Op::createRenderLayer));
    activate(opcast(Op::recordRenderCommand));
    activate(opcast(Op::render));
    activate(opcast(Op::saveSwapImage));
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
            case Op::render: pushState(&setRenderCommand); break;
            case Op::saveSwapImage: pushState(&saveSwapImage); break;
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

void RenderManager::receiveGraphicsPipelineReport(const GraphicsPipelineReport* r)
{
    for (const auto& renderLayerReport : renderLayersReports) 
    {
        if (renderLayerReport->getPipelineName() == r->getObjectName())
        {
            auto id = renderLayerReport->getId();
            for (const auto& renderCommandReport : renderCommandReports) 
            {
                if (renderCommandReport->containsRenderLayer(id))
                    rerecordRenderCommand(renderCommandReport.get());
            }
        }
    }
}

void RenderManager::rerecordRenderCommand(RenderCommandReport* report)
{
    auto cmd = rrcPool.request(
            OwningReportCallbackFn([this, report](Report* r) {
                auto cmdIndex = report->getCmdIndex();
                setRenderCommandF(0);
                }),
            report);
    pushCmd(std::move(cmd));
    std::cerr << "RenderManager::rerecordRenderCommand: pushed command" << '\n';
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

void RenderManager::setRenderCommandF(int cmdIndex)
{
    auto cmd = pool.request(cmdIndex);
    pushCmd(std::move(cmd));
}

} // namespace state

} // namespace sword


