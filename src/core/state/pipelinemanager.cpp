#include <state/pipelinemanager.hpp>

namespace sword
{

namespace state
{

CreateGraphicsPipeline::CreateGraphicsPipeline(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}, pool{sa.cp.createGraphicsPipeline}
{
    sa.rg.createGraphicsPipeline = this;
}

void CreateGraphicsPipeline::onEnterExt()
{
    std::cout << "Enter a name for the pipeline, then provide names for"
       " a pipelinelayout, a vertshader, a fragshader, "
       " a renderpass, 4 numbers for the render region,"
       " and a 1 or a 0 for whether or not this is a 3d or 2d pipeline" << std::endl;
}

void CreateGraphicsPipeline::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto name = ce->getArg<std::string, 0>();
        auto pipelineLayout = ce->getArg<std::string, 1>();
        auto vertshader = ce->getArg<std::string, 2>();
        auto fragshader = ce->getArg<std::string, 3>();
        auto renderpass = ce->getArg<std::string, 4>();
        vk::Rect2D area{
            {ce->getArg<int, 5>(), ce->getArg<int, 6>()},
            {ce->getArg<uint32_t, 7>(), ce->getArg<uint32_t, 8>()}};
        auto is3d = ce->getArg<bool, 9>();
        auto cmd = pool.request(reportCallback(), name, pipelineLayout,
                vertshader, fragshader, renderpass, area, is3d);
        pushCmd(std::move(cmd));
        popSelf();
        event->setHandled();
    }
}

CreatePipelineLayout::CreatePipelineLayout(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}, pool{sa.cp.createPipelineLayout}
{
    sa.rg.createPipelineLayout = this;
}

void CreatePipelineLayout::onEnterExt()
{
    std::cout << "Enter name a name for the pipeline layout and the names of the descriptor set layouts" << std::endl;
}

void CreatePipelineLayout::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);
        auto ss = ce->getStream();
        std::string name;
        std::string layout;
        std::vector<std::string> layouts;
        ss >> name;
        while (ss >> layout)
            layouts.push_back(layout);
        auto cmd = pool.request(reportCallback(), name, layouts);
        pushCmd(std::move(cmd));
        popSelf();
        event->setHandled();
    }
}

PipelineManager::PipelineManager(StateArgs sa, Callbacks cb) :
    BranchState{sa, cb, 
        {
            {"create_graphics_pipeline", opcast(Op::createGraphicsPipeline)},
            {"create_pipeline_layout", opcast(Op::createPipelineLayout)},
            {"print_reports", opcast(Op::printReports)}
        }
    },
    createPipelineLayout{sa, {
        nullptr,
        [this](Report* report){ 
            addReport<PipelineLayoutReport>(report, &pipeLayoutReports, [this](const PipelineLayoutReport* r){ receiveReport(r); }); }}},
        //must be explicit on function type because we are passing in a lambda in place of the std::function
    createGraphicsPipeline{sa, {
        nullptr,
        [this](Report* report){ addReport(report, &graphicsPipeReports); }}}
{
    activate(opcast(Op::createGraphicsPipeline));
    activate(opcast(Op::createPipelineLayout));
    activate(opcast(Op::printReports));
}

void PipelineManager::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto option = extractCommand(event);
        if (!option) return;
        switch (opcast<Op>(*option))
        {
            case Op::createGraphicsPipeline: pushState(&createGraphicsPipeline); event->setHandled(); break;
            case Op::createPipelineLayout: pushState(&createPipelineLayout); event->setHandled();  break;
            case Op::printReports: printReports(); break;
        }
    }
}

// look for issues here... 
void PipelineManager::receiveReport(const Report* pReport)
{
    reports.push_back(pReport);
    if (pReport->getType() == ReportType::DescriptorSetLayout)
        createPipelineLayout.addToVocab(pReport->getObjectName());
    if (pReport->getType() == ReportType::Shader)
        createGraphicsPipeline.addToVocab(pReport->getObjectName());
    if (pReport->getType() == ReportType::RenderPass)
        createGraphicsPipeline.addToVocab(pReport->getObjectName());
    if (pReport->getType() == ReportType::PipelineLayout)
        createGraphicsPipeline.addToVocab(pReport->getObjectName());
}

void PipelineManager::printReports()
{
    std::cout << "        PIPELINE MANAGER REPORTS" << std::endl;
    for (const auto& r : pipeLayoutReports) 
    {
        std::invoke(*r);
    }
    for (const auto& r : graphicsPipeReports) 
    {
        std::invoke(*r);
    }
}

}; // namespace state

}; // namespace sword
