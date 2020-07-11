#include <state/state.hpp>

namespace sword
{

namespace state
{

void State::pushCmd(command::Vessel cmd)
{
    cmdStack.push(std::move(cmd));
}

void State::onEnter()
{
    onEnterExt();
    onEnterImp();
}

void State::onExit()
{
    onExitExt();
    onExitImp();
    if (onExitCallback)
        std::invoke(onExitCallback);
}

void State::onEnterImp()
{
    auto cmd = avPool.request(&vocab);
    pushCmd(std::move(cmd));
    std::cout << "Commandline options: ";
    printVocab();
}

void State::onExitImp()
{
    auto cmd = pvPool.request();
    pushCmd(std::move(cmd));
}

void State::setVocab(std::vector<std::string> strings)
{
    vocab.set(strings);
}

void State::updateVocab()
{
    auto cmd = uvPool.request();
    cmdStack.push(std::move(cmd));
}

void State::printVocab()
{
    vocab.print();
}

std::vector<std::string> State::getVocab()
{
    return vocab.getWords();
}

void LeafState::onEnterImp()
{
    auto cmd = svPool.request(getVocab());
    pushCmd(std::move(cmd));
    std::cout << "Commandline options: ";
    printVocab();
}

void LeafState::onExitImp()
{
//    onExitExt();
    updateVocab();
}

void LeafState::pushCmd(command::Vessel cmd)
{
    cmdStack.push(std::move(cmd));
}

void BranchState::pushState(State* state)
{
    editStack.pushState(state);
}

Optional BranchState::extractCommand(event::Event* event)
{
    auto cmdevent = static_cast<event::CommandLine*>(event);
    std::string input = cmdevent->getFirstWord();
    return options.findOption(input, topMask); //TODO give this a bitmask parameter to filter the options by
}
//
//void BranchState::setActiveVocab()
//{
//    setVocab(options.getStrings(topMask));
//}
//
void BranchState::updateActiveVocab()
{
//    setActiveVocab();
    updateVocab();
}

void BranchState::printMask()
{
    std::cout << "Mask: ";
    std::cout << topMask.to_string() << std::endl;
}

}; //state

}; //sword

