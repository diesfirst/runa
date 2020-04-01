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
    auto cmd = pvPool.request();
    pushCmd(std::move(cmd));
}

void State::setVocab(std::vector<std::string> strings)
{
    vocab = strings;
    updateVocab();
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
    return options.findOption(input);
}

void BranchState::onPush()
{
    setActiveVocab();
}

void BranchState::onResume()
{
    filterOutVocab();
}

void BranchState::setActiveVocab()
{
    clearVocab();
    for (int i = 0; i < options.size(); i++) 
        if (topMask[i]) 
            addToVocab(options.stringAt(i));
    printVocab();
}

void BranchState::filterOutVocab()
{
    clearVocab();
    auto mask = lowMask & topMask;
    for (int i = 0; i < options.size(); i++) 
        if (mask[i]) 
            addToVocab(options.stringAt(i));
}

}; //state

}; //sword

