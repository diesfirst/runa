#include <state/director.hpp>
#include <event/event.hpp>
#include <util/stringutil.hpp>

namespace sword
{

namespace state
{

Director::Director(StateArgs sa, const StateStack& ss, render::Window& window) :
    BranchState{sa, Callbacks(), {
        {"render_manager", opcast(Op::pushRenderManager)},
        {"print_state_hierarchy", opcast(Op::printHierarchy)},
        {"quick_setup", opcast(Op::quickSetup)}
    }}, 
    stateStack{ss},
    renderManager{sa, 
        {[this](){activate(opcast(Op::pushRenderManager));}, {}, {}}},
    sr{sa.rg},
    cp{sa.cp}
{ 
    activate(opcast(Op::pushRenderManager));
    activate(opcast(Op::printHierarchy));
    activate(opcast(Op::quickSetup));
}

void Director::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        Optional option = extractCommand(event);
        if (!option) return;
        switch(opcast<Op>(*option))
        {
            case Op::printHierarchy: printStateHierarchy(); break;
            case Op::pushRenderManager: pushRenderManager(); break;
            case Op::quickSetup: quickSetup(); break;
        }
        event->setHandled();
    }
    if (event->getCategory() == event::Category::Abort)
    {
        popTop();
    }
}

void Director::printStateHierarchy()
{
    std::cout << util::makeHeader("State Stack") << std::endl;
    stateStack.print();
}

void Director::pushRenderManager()
{
    pushState(&renderManager);
    deactivate(opcast(Op::pushRenderManager));
}

void Director::popTop()
{
    if (stateStack.size() > 1)
        editStack.popState();
}

void Director::quickSetup()
{
    pushCmd(cp.loadFragShader.request(sr.loadFragShaders->reportCallback(), "death.spv"));
    pushCmd(cp.loadFragShader.request(sr.loadFragShaders->reportCallback(), "justice.spv"));
    deactivate(opcast(Op::quickSetup));
    updateVocab();
}

}; // namespace state

}; // namespace sword
