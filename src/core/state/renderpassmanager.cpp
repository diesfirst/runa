#include <state/renderpassmanager.hpp>

namespace sword
{

namespace state
{

CreateRenderPass::CreateRenderPass(StateArgs sa, Callbacks cb) :
    LeafState{sa, cb}, csrpPool{sa.cp.createSwapchainRenderpass}, corpPool{sa.cp.createOffscreenRenderpass}
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
        Report* report;
        CmdPtr cmd;
        if (type == RenderPassType::swapchain)
        {
            cmd = csrpPool.request(reportCallback(), name);
        }
        else if (type == RenderPassType::offscreen)
        {
            std::string strType = ce->getArg<std::string, 1>();
            vk::AttachmentLoadOp loadop;
            if (strType == "clear")
                loadop = vk::AttachmentLoadOp::eClear;
            else if (strType == "load") 
                loadop = vk::AttachmentLoadOp::eLoad;
            else 
            {
                std::cout << "bad load op" << std::endl;
                popSelf();
                event->setHandled();
                return;
            }
            cmd = corpPool.request(reportCallback(), name, loadop);
        }
        else
        {
            std::cout << "What?" << std::endl;
            popSelf();
            event->setHandled();
            return;
        }
        pushCmd(std::move(cmd));
        popSelf();
        event->setHandled();
    }
}

void CreateRenderPass::setType(RenderPassType t)
{
    type = t;
}

RenderPassManager::RenderPassManager(StateArgs sa, Callbacks cb, ReportCallbackFn<RenderPassReport> rpr) :
    BranchState{sa, cb,
        {
            {"create_swapchain_renderpass", opcast(Op::createSwapRpass)},
            {"create_offscreen_renderpass", opcast(Op::createOffscreenRpass)},
            {"print_reports", opcast(Op::printReports)}
        }
    },
    createRenderPass{sa, 
          {nullptr, [this, rpr](Report* ptr){ addReport(ptr, &reports, rpr); }}}
{
    activate(opcast(Op::createOffscreenRpass));
    activate(opcast(Op::printReports));
}

void RenderPassManager::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        auto option = extractCommand(event);
        if (!option) return;
        switch (opcast<Op>(*option))
        {
            case Op::createSwapRpass: pushCreateRenderPass(RenderPassType::swapchain); event->setHandled(); break;
            case Op::createOffscreenRpass: pushCreateRenderPass(RenderPassType::offscreen); event->setHandled(); break;
            case Op::printReports: printReports(); break;
        }
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

void RenderPassManager::printReports()
{
    std::cout << "Called print reports" << std::endl;
    for (const auto& r : reports) 
    {
        std::invoke(*r);
    }
}

}; // namespace state

}; // namespace sword
