#include <state/state.hpp>
#include <application.hpp>

namespace sword
{

namespace state
{

void State::pushCmd(CmdPtr cmd)
{
    cmdStack.push(std::move(cmd));
}

void State::onEnter(Application* app)
{
    onEnterExt(app);
    app->dispatcher.addVocab(vocab.getValues());
    std::cout << "Commandline options: ";
    for (const auto& i : vocab) 
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

void State::onExit(Application* app)
{
    onExitExt(app);
    app->dispatcher.popVocab();
}

void BranchState::pushState(State* state)
{
    editStack.pushState(state);
    activeChild = state;
}

Optional BranchState::extractCommand(event::Event* event)
{
    auto cmdevent = static_cast<event::CommandLine*>(event);
    std::string input = cmdevent->getFirstWord();
    return options.findOption(input);
}

}; //state

}; //sword

