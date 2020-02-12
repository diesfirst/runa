#include "commandstate.hpp"
#include "../core/util.hpp"
#include <sstream>
#include <filesystem>

#define SET_VOCAB()\
        std::vector<std::string> vocab;\
        for (auto cmd : cmds)\
            vocab.push_back(cmd->getName());\
        std::cout << std::endl;\
        app->setVocabulary(vocab)\
    
constexpr const char* SHADER_DIR = "/home/michaelb/Dev/runa/build/shaders/";

//TODO: we want to have states able to push states to stack based on input.
//then, when we get to a state where all of its commands can be specified
//solely on the next event, we run that command. so this is like a state tree.
//states can be transitioned to based on the current state and the event.
//should we allow commands to be run from states that can still transition state?
//yes. think of the director state. it needs to be able to transition to multiple
//states, but there are also commands that could be directly run from it. like a 
//loadDefaultAttachment() or the like. every leaf node in this state tree is a command.
//this will allow us to get rid of the command state variables (pending, stage) 
//and execute will JUST run the command. The commands ONLY members will be its arguments.
//this will mean that state popping will have to happen inside the handleEvent
//function. some states will need to "charge" a few times before they can return
//their command.
Command::Command* State::Director::handleEvent(Application* app, Event* event) const
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        std::string filtered;
        instream >> filtered;
        for (auto cmd : cmds) {
            if (cmd->getName() == filtered)
            {
                return cmd;
            }
        }
    }
    return nullptr;
}

Command::Command* State::AddAttachment::handleEvent(Application* app, Event* event) const
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        instream >> cmd->attachmentName >> cmd->isSampled >> cmd->isTransferSrc;
        return cmd;
    }
    return nullptr;
}

Command::Command* State::LoadShaders::handleEvent(Application* app, Event* event) const
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
        if (cmd->getStage() == 1)
        {
            std::string input; instream >> input;
            if (input == "vert")
                cmd->isFrag = false;
        }
        if (cmd->getStage() == 2)
        {
            std::vector<std::string> shaders;
            std::string input;
            while (instream >> input)
                shaders.push_back(input);
            cmd->shaderNames = shaders;
        }
        event->setHandled();
        return cmd;
    }
    return nullptr;
}

Command::Command* State::Paint::handleEvent(Application* app, Event* event) const
{
    if (event->getCategory() == EventCategory::Window)
    {
        auto e = static_cast<WindowEvent*>(event);
        if (e->getType() == WindowEventType::Motion)
        {
            auto me = static_cast<MouseMotionEvent*>(e);
            std::cout << me->getX() << std::endl;
            std::cout << me->getY() << std::endl;
        }
    }
    return nullptr;
}

void Application::setVocabulary(std::vector<std::string> vocab)
{
    ev.setVocabulary(vocab);
}

void State::Director::onEnter(Application* app) const
{
    SET_VOCAB();
}

void State::LoadShaders::onEnter(Application* app) const
{
    if (cmd->getStage() == 1)
    {
        std::vector<std::string> vocab = {"frag", "vert"};
        app->setVocabulary(vocab);
        std::cout << "Load fragment or vertex shaders?" << std::endl;
    }
    if (cmd->getStage() == 2)
    {
        std::vector<std::string> vocab;
        auto dir = std::filesystem::directory_iterator(SHADER_DIR);
        for (const auto& entry : dir) 
        {
            vocab.push_back(entry.path().filename());
        }
        app->setVocabulary(vocab);
        std::cout << "Choose which shaders to load." << std::endl;
    }
}

void State::AddAttachment::onEnter(Application* app) const 
{
    //we could set the vocab to nothing if we wanted
    std::cout << "Enter a name, 1 if it is to be sampled, 1 if it is to be a transfer src." << std::endl;
}

void State::Paint::onEnter(Application* app) const
{
}

void Command::AddAttachment::loadArgs(std::string attachmentName, bool isSampled, bool isTransferSrc)
{
    attachmentName = attachmentName;
    isSampled = isSampled;
    isTransferSrc = isTransferSrc;
}

State::State* Command::AddAttachment::execute(Application* app)
{
    if (!pending)
    {
        attachmentName = "default";
        isSampled = false;
        isTransferSrc = false;

        pending = true;
        return &nState;
    }
    else
    {
        vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
        if (isSampled)
            usage = usage | vk::ImageUsageFlagBits::eSampled;
        if (isTransferSrc)
            usage = usage | vk::ImageUsageFlagBits::eTransferSrc;
        app->renderer.createAttachment(
                attachmentName, app->offscreenDim, usage);
        app->popState();
        std::cout << "state popped" << std::endl;
        pending = false;
        return nullptr;
    }
}

State::State* Command::LoadShaders::execute(Application* app)
{
    if (stage < 2)
    {
        stage++;
        return &nState;
    }
    else
    {
        stage++; //just to prevent entering again
        std::cout << "Popping states off" << std::endl;
        app->popState();
        app->popState();
        stage = 0;
        return nullptr;
    }
}

State::State* Command::OpenWindow::execute(Application* app)
{
    app->window.open();
    return nullptr;
}

State::State* Command::EnterPaint::execute(Application* app)
{
    return state;
}

Application::Application(uint16_t w, uint16_t h) :
    context{},
    window{w, h},
    ev{window},
    renderer{context},
    dirState{{&aaCmd, &lsCmd, &ow, &epCmd}},
    paintState{{&stampCmd}},
    epCmd{&paintState},
    offscreenDim{{w, h}}
{
    stateStack.push(&dirState);
}

void Application::popState()
{
    stateStack.pop();
    //must call onEnter on all current states to roll back any changes
    //the popped one might have inflicted
    //note that since were using a vector this will call the onEnters in
    //the correct order
    for (auto state : stateStack.states) {
        state->onEnter(this);
    }
}

void Application::run()
{
    stateStack.top()->onEnter(this);
    ev.pollEvents();

    while (1)
    {
        while (!ev.eventQueue.empty())
        {
            auto event = ev.eventQueue.front().get();
            if (event)
                for (auto state : stateStack.states) 
                    if (!event->isHandled())
                        cmdStack.push(state->handleEvent(this, event));
            ev.eventQueue.pop();
        }
        while (!cmdStack.empty())
        {
            auto cmd = cmdStack.top();
            if (cmd)
            {
                State::State* state = cmd->execute(this);
                if (state)
                {
                    stateStack.push(state);
                    state->onEnter(this);
                }
            }
            cmdStack.pop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

