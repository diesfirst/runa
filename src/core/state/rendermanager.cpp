#include <state/rendermanager.hpp>

namespace sword
{

namespace state
{

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
            case Op::shaderManager: pushState(&shaderManager); break;
        }
    }
}

void RenderManager::openWindow()
{
    auto cmd = owPool.request();
    pushCmd(std::move(cmd));
    deactivate(opcast(Op::openWindow));
    updateActiveVocab();
}

void RenderManager::prepRenderFrames()
{
    auto cmd = prfPool.request();
    pushCmd(std::move(cmd));
    deactivate(opcast(Op::prepRenderFrames));
    updateActiveVocab();
}

}; // namespace state

}; // namespace sword


