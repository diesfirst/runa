#include <state/state.hpp>

namespace sword
{

namespace state
{

void State::pushCmd(CmdPtr cmd)
{
    cmdStack.push(std::move(cmd));
}

void State::onEnter()
{
    onEnterExt();
    auto cmd = avPool.request(&vocab);
    pushCmd(std::move(cmd));
    std::cout << "Commandline options: ";
    printVocab();
}

void State::onExit()
{
    onExitExt();
    if (onExitCallback)
        std::invoke(onExitCallback);
    auto cmd = pvPool.request();
    pushCmd(std::move(cmd));
}

void State::setVocab(std::vector<std::string> strings)
{
    vocab = strings;
}

void State::updateVocab()
{
    auto cmd = uvPool.request();
    cmdStack.push(std::move(cmd));
}

void State::printVocab()
{
    for (const auto& i : vocab) 
        std::cout << i << " ";
    std::cout << std::endl;
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
    if (active)
        return options.findOption(input, topMask); //TODO give this a bitmask parameter to filter the options by
    else
        return options.findOption(input, topMask & lowMask);
}

void BranchState::onPush()
{
    filterOutVocab();
    active = false;
}

void BranchState::onResume()
{
    setActiveVocab();
    active = true;
}

void BranchState::setActiveVocab()
{
    setVocab(options.getStrings(topMask));
}

void BranchState::filterOutVocab()
{
    setVocab(options.getStrings(topMask & lowMask));
}

void BranchState::updateActiveVocab()
{
    setActiveVocab();
    updateVocab();
}

}; //state

}; //sword

