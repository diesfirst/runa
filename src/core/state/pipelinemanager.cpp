#include <state/pipelinemanager.hpp>

namespace sword
{

namespace state
{

CreateGraphicsPipeline::CreateGraphicsPipeline(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}
{}

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
        std::cout << "Meh" << std::endl;       
    }
}

CreatePipelineLayout::CreatePipelineLayout(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}
{}

void CreatePipelineLayout::onEnterExt()
{
    std::cout << "Enter name a name for the pipeline layout and the names of the descriptor set layouts" << std::endl;
}

void CreatePipelineLayout::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        std::cout << "Gooo" << std::endl;       
    }
}

PipelineManager::PipelineManager(StateArgs sa, Callbacks cb) :
    BranchState{sa, cb, 
        {
            {"create_graphics_pipeline", opcast(Op::createGraphicsPipeline)},
            {"create_pipeline_layout", opcast(Op::createPipelineLayout)}
        }
    },
    createPipelineLayout{sa, {}},
    createGraphicsPipeline{sa, {}}
{
    activate(opcast(Op::createGraphicsPipeline));
    activate(opcast(Op::createPipelineLayout));
}


void PipelineManager::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto option = extractCommand(event);
        if (!option) return;
        switch (opcast<Op>(*option))
        {
            case Op::createGraphicsPipeline: break;
            case Op::createPipelineLayout: break;
        }
    }
}

}; // namespace state

}; // namespace sword
