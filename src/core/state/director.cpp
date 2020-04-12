#include <state/director.hpp>
#include <event/event.hpp>
#include <util/stringutil.hpp>

namespace sword
{

namespace state
{

void Director::handleEvent(event::Event* event)
{
    if (event->getCategory() == event::Category::CommandLine)
    {
        Optional option = extractCommand(event);
        if (!option) return;
        switch(opcast(*option))
        {
            case Op::foo: foo(); break;
            case Op::jim: jim(); break;
            case Op::printHierarchy: printStateHierarchy(); break;
            case Op::pushRenderManager: pushRenderManager(); break;
        }
        event->setHandled();
    }
    if (event->getCategory() == event::Category::Abort)
    {
        popTop();
    }
}

void Director::foo() 
{
    std::cout << "Foo boy!" << std::endl;
}

void Director::jim()
{
    std::cout << "Fuck you Jim, fuck you." << std::endl;
}

void Director::printStateHierarchy()
{
    std::cout << util::makeHeader("State Stack") << std::endl;
    stateStack.print();
}

void Director::pushRenderManager()
{
    pushState(&renderManager);
}

void Director::popTop()
{
    if (stateStack.size() > 1)
        editStack.popState();
}

}; // namespace state

}; // namespace sword
