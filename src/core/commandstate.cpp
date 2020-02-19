#include "commandstate.hpp"
#include "../core/util.hpp"
#include <sstream>
#include <filesystem>
#include <fstream>
#include <algorithm>

#define SET_VOCAB()\
        std::vector<std::string> vocab;\
        for (auto cmd : cmds)\
            vocab.push_back(cmd->getName());\
        std::cout << std::endl;\
        app->setVocabulary(vocab)\
    
constexpr const char* SHADER_DIR = "/home/michaelb/Dev/sword/build/shaders/";


void State::Director::handleEvent(Event* event, StateStack* stateEdits, CommandStack* cmdStack)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        std::string filtered;
        instream >> filtered;
        pushStateFn(filtered, addAttachState, stateEdits);
        pushStateFn(filtered, loadShaders, stateEdits);
        pushStateFn(filtered, initRenState, stateEdits);
        if (filtered == owPool.getName())
        {
            auto cmd = owPool.request();
            cmdStack->push(std::move(cmd));
        };
        event->isHandled();
    }
}

void State::AddAttachment::handleEvent(Event* event, StateStack* stateEdits, CommandStack* cmdStack)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        std::cout << "AddAttachment pushed" << std::endl;
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        std::string attachmentName; bool isSampled; bool isTransferSrc;
        instream >> attachmentName >> isSampled >> isTransferSrc;
        auto cmd = addAttPool.request(attachmentName, isSampled, isTransferSrc);
        stateEdits->push(nullptr);
        cmdStack->push(std::move(cmd));
        event->setHandled();
    }
}

void State::LoadShaders::handleEvent(Event* event, StateStack* stateEdits, CommandStack* cmdStack)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
        std::vector<std::string> shaders;
        std::string input;
        while (instream >> input)
        {
            auto cmd = loadFragPool.request(input);
            cmdStack->push(std::move(cmd));
        }
        stateEdits->push(nullptr);
        event->setHandled();
    }
}

void State::InitRenderer::handleEvent(Event* event, StateStack* stateEdits, CommandStack* cmdStack)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
        std::string input; instream >> input;
        auto prepframes = [&]() mutable 
        {
            if (preparedFrames)
                std::cout << "Frames prepared already" << std::endl;
            else if (window.isOpen())
            {
                auto cmd = prfPool.request(&window);
                cmdStack->push(std::move(cmd));
                preparedFrames = true;
            }
            else
                std::cout << "window must be open" << std::endl;
        };
        auto createlayout = [&]() mutable 
        {
            if (createdLayout)
                std::cout << "Already created layout" << std::endl;
            else
            {
                auto cmd = cplPool.request();
                cmdStack->push(std::move(cmd));
                createdLayout = true;
            }
        };
        if (input == prfPool.getName())
        {
            prepframes();
            event->setHandled();
            stateEdits->push(nullptr);
        }
        else if (input == cplPool.getName())
        {
            stateEdits->push(nullptr);
            event->setHandled();
            createlayout();
        }
        else if (input == "all")
        {
            prepframes();
            createlayout();
            stateEdits->push(nullptr);
            event->setHandled();
        }
    }
}

void State::Paint::handleEvent(Event* event, StateStack* stateEdits, CommandStack* cmdStack)
{
    std::cout << "oooh" << std::endl;
}

void State::Director::onEnter(Application* app) 
{
    app->setVocabulary(vocab);
}

void State::LoadShaders::onEnter(Application* app) 
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
    app->setVocabulary(vocab);
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

void Command::LoadFragShader::execute(Application* app)
{
    app->renderer.loadFragShader(SHADER_DIR + shaderName, shaderName);
}

void Command::LoadVertShader::execute(Application* app)
{
    app->renderer.loadVertShader(SHADER_DIR + shaderName, shaderName);
}

void Command::OpenWindow::execute(Application* app)
{
    app->window.open();
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

Application::Application(uint16_t w, uint16_t h, bool rec, bool rea) :
    window{w, h},
    ev{window},
    renderer{context},
    offscreenDim{{w, h}},
    swapDim{{w, h}},
    recordevents{rec},
    readevents{rea},
    dirState{window}
{
    stateStack.push(&dirState);
}

void Application::popState()
{
    stateStack.pop();
    stateStack.top()->onEnter(this);
}

void Application::pushState(State::State* state)
{
    stateStack.push(std::move(state));
    state->onEnter(this);
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


constexpr const char* eventlog = "eventlog";

void recordEvent(Event* event, std::ofstream& os)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        os.open(eventlog, std::ios::out | std::ios::binary | std::ios::app);
        auto ce = static_cast<CommandLineEvent*>(event);
        ce->serialize(os);
        os.close();
    }
}

void Application::readEvents(std::ifstream& is)
{
    is.open(eventlog, std::ios::binary);
    bool reading{true};
    while (is.peek() != EOF)
    {
        auto cmdevnt = std::make_unique<CommandLineEvent>("foo");
        cmdevnt->unserialize(is);
        ev.eventQueue.push(std::move(cmdevnt));
    }
    is.close();
}

void Application::run()
{
    stateStack.top()->onEnter(this);
    ev.pollEvents();
    std::ofstream os;
    std::ifstream is;
    
    if (readevents) readEvents(is);
    if (recordevents) std::remove(eventlog);

    while (1)
    {
        while (!ev.eventQueue.empty())
        {
            auto event = ev.eventQueue.front().get();
            if (recordevents) recordEvent(event, os);
            if (event)
                for (auto state : stateStack) 
                    if (!event->isHandled())
                        state->handleEvent(event, &stateEdits, &cmdStack);
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
            for (auto& cmd : cmdStack.items)
            {
                if (cmd)
                {
                    cmd->execute(this);
                    sleep(1);
                    std::cout << "Command run" << std::endl;
                }
                else
                    std::cout << "Recieved null cmd" << std::endl;
            }
            cmdStack.items.clear();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

