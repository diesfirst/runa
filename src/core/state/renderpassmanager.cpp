#include <state/renderpassmanager.hpp>

namespace sword
{

namespace state
{

CreateRenderPass::CreateRenderPass(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}
{}

void CreateRenderPass::onEnterExt()
{
    std::cout << "Enter a name and optionally load or clear (offscreen only)" << std::endl;
}

void CreateRenderPass::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto ce = toCommandLine(event);       
        auto name = ce->getFirst<std::string>();
        CmdPtr cmd;
        if (type == RenderPassType::swapchain)
            cmd = csrpPool.request(name);
        else if (type == RenderPassType::offscreen)
        {
            std::string strType = ce->getArg<std::string, 1>();
            vk::AttachmentLoadOp loadop;
            if (strType == "swap")
                loadop = vk::AttachmentLoadOp::eClear;
            if (strType == "clear") 
                loadop = vk::AttachmentLoadOp::eLoad;
            cmd = corpPool.request(name, loadop);
        }
        else
        {
            std::cout << "What?" << std::endl;
            popSelf();
            return;
        }
        pushCmd(std::move(cmd));
        popSelf();
    }
}

void CreateRenderPass::setType(RenderPassType t)
{
    type = t;
}

RenderPassManager::RenderPassManager(StateArgs sa, Callbacks cb) :
    BranchState{sa, cb,
        {
            {"create_swapchain_renderpass", opcast(Op::createSwapRpass)},
            {"create_offscreen_renderpass", opcast(Op::createOffscreenRpass)}
        }
    },
    createRenderPass{sa, 
        {nullptr, std::bind(&BranchState::addReport<RenderPassReport>, this, std::placeholders::_1, &reports)}}
{
    activate(opcast(Op::createOffscreenRpass));
}

void RenderPassManager::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto option = extractCommand(event);
        if (!option) return;
        switch (opcast<Op>(*option))
        {
            case Op::createSwapRpass: pushCreateRenderPass(RenderPassType::swapchain); break;
            case Op::createOffscreenRpass: pushCreateRenderPass(RenderPassType::offscreen); break;
        }
        event->setHandled();
    }
}

void RenderPassManager::pushCreateRenderPass(RenderPassType type)
{
    createRenderPass.setType(type);
    pushState(&createRenderPass);
}

void RenderPassManager::activateCreateSwap()
{
    activate(opcast(Op::createSwapRpass));
}

}; // namespace state

}; // namespace sword
