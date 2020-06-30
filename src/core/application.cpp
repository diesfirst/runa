#include <application.hpp>
#include <cstdint>
#include <state/director.hpp>
#include <event/event.hpp>
#include <state/state.hpp>
#include <thread>
#include <util/debug.hpp>

namespace sword
{

constexpr std::uint16_t windowWidth{800};
constexpr std::uint16_t windowHeight{800};

//TODO: we should not require window be initialized here. initialization of a window should be a command
Application::Application(bool validation) :
    context{validation},
    window{windowWidth, windowHeight},
    dispatcher{window},
    renderer{context},
    offscreenDim{{windowWidth, windowHeight}},
    swapDim{{windowWidth, windowHeight}},
    dirState{{stateEdits, cmdStack, cmdPools, stateRegister, context}, stateStack, window}
{
}

Application::Application(uint16_t w, uint16_t h, const std::string logfile, int event_reads) :
    context{true},
    window{w, h},
    dispatcher{window},
    renderer{context},
    offscreenDim{{w, h}},
    swapDim{{w, h}},
    readlog{logfile},
    dirState{{stateEdits, cmdStack, cmdPools, stateRegister, context}, stateStack, window}
{
    stateStack.push(&dirState);
    stateStack.top()->onEnter();

    if (readlog != "eventlog")
    {
        std::cout << "reading events from " << readlog << std::endl;
        recordevents = true;
        readevents = true;
    }
    //if (readevents) readEvents(is, eventPops);
    if (recordevents) std::remove(writelog.data());

    if (event_reads == 0)
        maxEventReads = INT32_MAX;
    else 
        maxEventReads = event_reads;

}

void Application::popState()
{
    stateStack.top()->onExit();
    stateStack.pop();
}

void Application::pushState(state::State* state)
{
    stateStack.push(std::move(state));
    if (state->getType() != state::StateType::leaf)
        state->onEnter();
}

void Application::pushCmd(CmdPtr&& command)
{
    cmdStack.push(std::move(command));
}

void Application::recordEvent(event::Event* event, std::ofstream& os)
{
    if (event->getCategory() == event::Category::CommandLine)
    {

        os.open(writelog, std::ios::out | std::ios::binary | std::ios::app);
        auto ce = static_cast<event::CommandLine*>(event);
        ce->serialize(os);
        os.close();
    }
    if (event->getCategory() == event::Category::Abort)
    {

        os.open(writelog, std::ios::out | std::ios::binary | std::ios::app);
        auto ce = static_cast<event::Abort*>(event);
        ce->serialize(os);
        os.close();
    }
}

void Application::readEvents(std::ifstream& is, int eventPops)
{
    is.open(readlog, std::ios::binary);
    dispatcher.readEvents(is, eventPops);
    is.close();
}

void Application::drainEventQueue()
{
    int i = 0;
    dispatcher.lock.lock();
    while (i < dispatcher.eventQueue.size())
    {
        auto& event = dispatcher.eventQueue.items[i];
        if (event)
        {
            if (recordevents) recordEvent(event.get(), os);

            for (auto state : stateStack) 
            {
                state->handleEvent(event.get());
                if (event->isHandled()) 
                    break;
            }
        }

        if (stateEdits.size() > 0)
        {
            int j = 0;
            while (j < stateEdits.size())
            {
                auto state = stateEdits.at(j);
                if (state)
                    pushState(state);
                else
                    popState();
                j++;
            }
            stateEdits.clear();
            // this allows mutiple leaves to be pushed at once, and they get entered in order
            if (stateStack.top()->getType() == state::StateType::leaf)
                stateStack.top()->onEnter();
        }
        i++;
    }
    dispatcher.eventQueue.items.clear();
    dispatcher.lock.unlock();
}

void Application::executeCommands()
{
    if (!cmdStack.empty())
    {
        cmdStack.reverse();
    }
    while (!cmdStack.empty())
    {
        auto cmd = cmdStack.topPtr();
        if (cmd)
        {
            cmd->execute(this);
        }
        else
            std::cout << "Recieved null cmd" << std::endl;
        cmdStack.pop();
        if (cmd && cmd->succeeded())
            cmd->onSuccess();
    }
    cmdStack.items.clear();
}

void Application::beginFrame()
{
    dirState.beginFrame();
}

void Application::endFrame()
{
    dirState.endFrame();
}

void Application::run(bool pollEvents)
{
    if (pollEvents)
    {
        dispatcher.pollEvents();
    }

    if (readevents)
        is.open(readlog, std::ios::binary);

    bool keepRunnning = true;
    while (keepRunnning)
    {
        if (!pollEvents)
            keepRunnning = false; //only runs once
        if (readevents && eventsRead < maxEventReads)
        {
            dispatcher.readEvent(is);
            eventsRead++;
        }

        beginFrame();

        drainEventQueue();

        endFrame();

        executeCommands();

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    if (readevents)
        is.close();
}

// in case you forget. we need to be able to handle cmdPtrs that end up pushing 
// new commands to the stack when they get destructed (returned the pool). so using a stack for the commands
// makes sense. but we also want the execution order to match the order in which the
// commands were pushed. so we need to reverse the stack before we start iterating over it
// this will be interesting to profile. i don't know yet how big the cmdStack is likely to get in practice.
// and whether iteraticing over it effectively twice will make a difference ultimately.


}; // namespace sword
