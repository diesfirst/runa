#include "commandstate.hpp"
#include "../core/util.hpp"
#include <sstream>
#include <filesystem>
#include <fstream>

#define SET_VOCAB()\
        std::vector<std::string> vocab;\
        for (auto cmd : cmds)\
            vocab.push_back(cmd->getName());\
        std::cout << std::endl;\
        app->setVocabulary(vocab)\
    
constexpr const char* SHADER_DIR = "/home/michaelb/Dev/runa/build/shaders/";

Command::Command* State::Director::handleEvent(Event* event, Stack<State*>* stateEdits)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        std::string filtered;
        instream >> filtered;
        for (auto state : states)
            if (state->getName() == filtered)
            {
                event->setHandled();
                stateEdits->push(state);
                return nullptr;
            }
        for (auto cmd : cmds) 
            if (cmd->getName() == filtered)
            {
                event->setHandled();
                return cmd;
            }
    }
    return nullptr;
}

Command::Command* State::AddAttachment::handleEvent(Event* event, Stack<State*>* stateEdits)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        std::cout << "AddAttachment pushed" << std::endl;
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        instream >> cmd->attachmentName >> cmd->isSampled >> cmd->isTransferSrc;
        stateEdits->push(nullptr);
        event->setHandled();
        return cmd;
    }
    return nullptr;
}

Command::Command* State::LoadFragShaders::handleEvent(Event* event, Stack<State*>* stateEdits)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
        std::vector<std::string> shaders;
        std::string input;
        while (instream >> input)
            shaders.push_back(input);
        cmd->shaderNames = shaders;
        stateEdits->push(nullptr);
        event->setHandled();
        return cmd;
    }
    return nullptr;
}

Command::Command* State::Paint::handleEvent(Event* event, Stack<State*>* stateEdits)
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

Command::Command* State::SetOffscreenDim::handleEvent(Event* event, Stack<State*>* stateEdits)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
        instream >> cmd->w >> cmd->h;
        stateEdits->push(nullptr);
        event->setHandled();
        return cmd;
    }
    return nullptr;
}

Command::Command* State::InitRenderer::handleEvent(Event* event, Stack<State*>* stateEdits)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
        std::string input; instream >> input;
        if (input == prepRFrames.getName())
        {
            if (preparedFrames)
            {
                std::cout << "Frames prepared already" << std::endl;
                return nullptr;
            }
            else if (window.isOpen())
            {
                prepRFrames.window = &window;
                stateEdits->push(nullptr);
                event->setHandled();
                preparedFrames = true;
                return &prepRFrames;
            }
            else
            {
                std::cout << "window must be open" << std::endl;
                stateEdits->push(nullptr);
                return nullptr;
            }
        }
    }
    return nullptr;
}

void State::Director::onEnter(Application* app) 
{
    std::vector<std::string> vocab;
    for (auto cmd : cmds)
        vocab.push_back(cmd->getName());
    for (auto st : states)
        vocab.push_back(st->getName());
    std::cout << std::endl;
    app->setVocabulary(vocab);
}

void State::LoadFragShaders::onEnter(Application* app) 
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

void State::AddAttachment::onEnter(Application* app) 
{
    //we could set the vocab to nothing if we wanted
    std::cout << "Enter a name, 1 if it is to be sampled, 1 if it is to be a transfer src." << std::endl;
}

void State::Paint::onEnter(Application* app)
{
}

void State::InitRenderer::onEnter(Application* app)
{
    std::vector<std::string> vocab;
    for (auto cmd : cmds)
        vocab.push_back(cmd->getName());
    std::cout << std::endl;
    app->setVocabulary(vocab);
}

void State::SetOffscreenDim::onEnter(Application* app)
{
}

void Command::AddAttachment::loadArgs(std::string attachmentName, bool isSampled, bool isTransferSrc)
{
    attachmentName = attachmentName;
    isSampled = isSampled;
    isTransferSrc = isTransferSrc;
}

void Command::AddAttachment::execute(Application* app)
{
        vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
        if (isSampled)
            usage = usage | vk::ImageUsageFlagBits::eSampled;
        if (isTransferSrc)
            usage = usage | vk::ImageUsageFlagBits::eTransferSrc;
        app->renderer.createAttachment(
                attachmentName, app->offscreenDim, usage);
}

void Command::LoadFragShaders::execute(Application* app)
{
    for (auto& shaderName : shaderNames) 
    {
        auto& shader = app->renderer.loadFragShader(SHADER_DIR + shaderName, shaderName);
        shader.setWindowResolution(app->offscreenDim.width, app->offscreenDim.height);
    }
}

void Command::OpenWindow::execute(Application* app)
{
    app->window.open();
}

void Command::SetOffscreenDim::execute(Application* app)
{
    app->offscreenDim.setWidth(w);
    app->offscreenDim.setHeight(h);
}

void Command::PrepareRenderFrames::execute(Application* app)
{
    app->renderer.prepareRenderFrames(*window);
    std::cout << "Render frames prepared" << std::endl;
}

void Command::CreatePipelineLayout::execute(Application* app)
{
    auto paintingLayout = app->renderer.createDescriptorSetLayout("paintLayout", bindings);
    app->renderer.createFrameDescriptorSets({paintingLayout}); //order matters for access
    auto paintPLayout = app->renderer.createPipelineLayout(layoutname, {paintingLayout});
    std::cout << "Created Pipeline Layout: " << layoutname << std::endl;
}

Command::CreatePipelineLayout::CreatePipelineLayout()
{
    bindings.resize(2);
    bindings[0].setBinding(0);
    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    bindings[0].setDescriptorCount(1);
    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    bindings[1].setBinding(1);
    bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    bindings[1].setDescriptorCount(1); //arbitrary
    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);
}

Application::Application(uint16_t w, uint16_t h) :
    window{w, h},
    ev{window},
    renderer{context},
    lsState{&lsCmd},
    aaState{&aaCmd},
    sodimState{&sodimCmd},
    dirState{{&initRen, &aaState, &lsState, &paintState, &sodimState}, {&ow}},
    paintState{{&stampCmd}},
    offscreenDim{{w, h}}
{
    stateStack.push(&dirState);
}

void Application::popState()
{
    stateStack.pop();
    stateStack.top()->onEnter(this);
}

void Application::pushState(State::State* const state)
{
    stateStack.push(state);
    state->onEnter(this);
}

void Application::pushCmd(Command::Command* const cmd)
{
    cmdStack.push(cmd);
}

void Application::setVocabulary(std::vector<std::string> vocab)
{
    ev.setVocabulary(vocab);
}

void Application::createPipelineLayout()
{
    const std::string layoutName = "layout";
    std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
    bindings[0].setBinding(0);
    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    bindings[0].setDescriptorCount(1);
    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    bindings[1].setBinding(1);
    bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    bindings[1].setDescriptorCount(1); //arbitrary
    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    auto paintingLayout = renderer.createDescriptorSetLayout("paintLayout", bindings);
    renderer.createFrameDescriptorSets({paintingLayout}); //order matters for access
    auto paintPLayout = renderer.createPipelineLayout(layoutName, {paintingLayout});
    std::cout << "Created Pipeline Layout: " << layoutName << std::endl;
}

void Application::loadDefaultShaders()
{
    auto& offScreenLoadPass = renderer.createRenderPass("offscreen_load");
    renderer.prepareAsOffscreenPass(offScreenLoadPass, vk::AttachmentLoadOp::eLoad); //can make this a static function of RenderPass
    auto& swapchainPass = renderer.createRenderPass("swapchain");
    renderer.prepareAsSwapchainPass(swapchainPass); //can make this a static function of RenderPass
    auto& offScreenClearPass = renderer.createRenderPass("offscreen_clear");
    renderer.prepareAsOffscreenPass(offScreenClearPass, vk::AttachmentLoadOp::eClear);
}

void Application::initUBO()
{
    renderer.initFrameUBOs(sizeof(FragmentInput), 0); //should be a vector
    renderer.bindUboData(static_cast<void*>(&fragInput), sizeof(FragmentInput), 0);
}

void recordEvent(Event* event, std::ofstream& os)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        os.open("eventrecord", std::ios::out | std::ios::binary | std::ios::app);
        auto ce = static_cast<CommandLineEvent*>(event);
        ce->serialize(os);
        os.close();
    }
}

void Application::readEvents(std::ifstream& is)
{
    is.open("eventrecord", std::ios::binary);
    bool reading{true};
    while (is.peek() != EOF)
    {
        auto cmdevnt = std::make_unique<CommandLineEvent>("foo");
        cmdevnt->unserialize(is);
        ev.eventQueue.push(std::move(cmdevnt));
    }
    is.close();
}

constexpr int record{0};
constexpr int readin{0};

void Application::run()
{
    stateStack.top()->onEnter(this);
    ev.pollEvents();
    std::ofstream os;
    std::ifstream is;
    
    if (readin) readEvents(is);

    while (1)
    {
        while (!ev.eventQueue.empty())
        {
            auto event = ev.eventQueue.front().get();
            if (record) recordEvent(event, os);
            if (event)
                for (auto state : stateStack.items) 
                    if (!event->isHandled())
                    {
                        auto cmd = state->handleEvent(event, &stateEdits);
                        if (cmd)
                            pushCmd(cmd);
                    }
            for (auto state : stateEdits.items) 
            {
                if (state)
                    pushState(state);
                else
                    popState();
            }
            stateEdits.items.clear();
            ev.eventQueue.pop();
        }
        if (!cmdStack.empty())
        {
            for (auto cmd : cmdStack.items)
            {
                cmd->execute(this);
                sleep(1);
                std::cout << "Command run" << std::endl;
            }
            cmdStack.items.clear();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

