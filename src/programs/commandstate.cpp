#include "commandstate.h"
#include "../core/util.hpp"

#define SET_VOCAB()\
        std::vector<std::string> vocab;\
        for (auto cmd : cmds)\
            vocab.push_back(cmd->getName());\
        std::cout << std::endl;\
        app->setVocabulary(vocab)\
    
namespace Cmd
{
    ReportStuff report;
    PrintWee pw;
};

namespace State
{
    InitialState init;
    FooState foo;
};

Command* InitialState::handleEvent(Application* app, Event* event) const
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        for (auto cmd : cmds) {
            if (cmd->getName() == input)
                return cmd;
        }
    }
    return nullptr;
}

Command* FooState::handleEvent(Application* app, Event* event) const
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        for (auto cmd : cmds) {
            if (cmd->getName() == input)
                return cmd;
        }
    }
    return nullptr;
}

void Application::setVocabulary(std::vector<std::string> vocab)
{
    ev.setVocabulary(vocab);
}

void InitialState::onEnter(Application* app) const
{
    SET_VOCAB();
}

void FooState::onEnter(Application* app) const
{
    SET_VOCAB();
}

void LoadShaderState::onEnter(Application* app) const
{
}

const AppState* ReportStuff::nextState() const
{
    return &State::foo;
}

const AppState* PrintWee::nextState() const
{
    return &State::init;
}

void ReportStuff::execute(Application* app)
{
    std::cout << "Report:" << std::endl;
}

void PrintWee::execute(Application* app)
{
    std::cout << "Weeee" << std::endl;
}

std::vector<Command*> InitialState::cmds = {&Cmd::report, &Cmd::pw};
std::vector<Command*> FooState::cmds = {&Cmd::report, &Cmd::pw};

void Application::run()
{
    state = &State::init;
    state->onEnter(this);
    ev.pollEvents();

    int i = 0;

    while (1)
    {
        while (!ev.eventQueue.empty())
        {
            std::cout << "Events left: " << ev.eventQueue.size() << std::endl;
            auto event = ev.eventQueue.front().get();
            if (event)
            {
                auto cmd = state->handleEvent(this, event);
                if (cmd)
                {
                    std::cout << "got a command." << std::endl;
                    cmd->execute(this);
                    state = cmd->nextState();
                    if (state)
                        state->onEnter(this);
                }
            }
            ev.eventQueue.pop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main(int argc, const char *argv[])
{
    XWindow window{500, 500};   
    EventHandler ev{window};
    window.open();
    Application app{ev};

    app.run();
    return 0;
}
