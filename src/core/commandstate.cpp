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

void State::Director::handleEvent(Event* event, EditStack* stateEdits, CommandStack* cmdStack)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        instream >> input;
        auto iter = opMap.find(input);
        Option option;
        if (iter != opMap.end())
                option = iter->second;
        else 
            option = Option::null;
        switch (option)
        {
            case Option::initRenState:
                {
                    stateEdits->push(&initRenState);
                    break;
                }
            case Option::null:
                {
                    break;
                }

        }
        event->isHandled();
    }
}

void State::AddAttachment::handleEvent(Event* event, EditStack* stateEdits, CommandStack* cmdStack)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        switch (mode)
        {
            case Option::null:
                {
                    instream >> input;
                    auto iter = vocab.find(input);
                    Option option;
                    if (iter != vocab.end())
                            option = iter->second;
                    else 
                        option = Option::null;
                    switch (option)
                    {
                        case Option::setdimensions:
                            {
                                std::cout << "Enter a width and height" << std::endl;
                                mode = Option::setdimensions;
                                break;
                            }
                        case Option::fulldescription:
                            {
                                std::cout << "Enter a name, a width, a height, is sampled, is transfer src" << std::endl;
                                mode = Option::fulldescription;
                                break;
                            }
                        case Option::addsamplesources:
                            {
                                std::cout << "enter names for attachments that will be sampled from" << std::endl;
                                mode = Option::addsamplesources;
                                break;
                            }
                        case Option::null:
                            {
                                stateEdits->push(nullptr);
                                break;
                            }
                    }
                    break;
                }
            case Option::setdimensions:
                {
                    instream >> width >> height;
                    mode = Option::null;
                    break;
                };
            case Option::addsamplesources:
                {
                    while (instream >> input)
                    {
                        auto cmd = addAttPool.request(input, width, height, true, false);
                        cmdStack->push(std::move(cmd));
                    }
                    mode = Option::null;
                    stateEdits->push(nullptr);
                    break;
                }
            case Option::fulldescription:
                {
                    std::string name; int w; int h; bool sample; bool transfersrc;
                    instream >> name >> w >> h >> sample >> transfersrc;
                    auto cmd = addAttPool.request(name, w, h, sample, transfersrc);
                    cmdStack->push(std::move(cmd));
                    mode = Option::null;
                    stateEdits->push(nullptr);
                    break;
                }
        }
        event->setHandled();
    }
}

void State::ShaderManager::handleEvent(Event* event, EditStack* stateEdits, CommandStack* cmdStack)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        switch (mode)
        {
            case Option::null:
                {
                    instream >> input;
                    auto iter = opMap.find(input);
                    Option option;
                    if (iter != opMap.end())
                            option = iter->second;
                    else 
                        option = Option::null;
                    switch (option)
                    {
                        case Option::null:
                            {
                                stateEdits->push(nullptr);
                                break;
                            }
                        case Option::loadFragShaders:
                            {
                                mode = Option::loadFragShaders;
                                std::cout << "Choose which frag shaders to load." << std::endl;
                                stateEdits->push(this);
                                break;
                            }
                        case Option::loadVertShaders:
                            {
                                mode = Option::loadVertShaders;
                                std::cout << "Choose which vert shaders to load." << std::endl;
                                stateEdits->push(this);
                                break;
                            }
                        case Option::shaderReport:
                            {
                                for (const auto& item : shaderReports) 
                                    std::invoke(item.second);
                                break;
                            }
                        case Option::setSpecFloats:
                            {
                                mode = Option::setSpecFloats;
                                std::cout << "Enter two floats, v or f, and the names of the shaders to set" << std::endl;
                                stateEdits->push(this);
                                break;
                            }
                        case Option::setSpecInts:
                            {
                                mode = Option::setSpecInts;
                                std::cout << "Enter two ints, v or f, and the names of the shaders to set" << std::endl;
                                stateEdits->push(this);
                                break;
                            }
                    }
                    break;
                }
            case Option::loadFragShaders:
                {
                    while (instream >> input)
                    {
                        auto cmd = loadFragPool.request(input);
                        cmdStack->push(std::move(cmd));
                        shaderReports.try_emplace(input, input, "Fragment", 0, 0, 0, 0);
                        shaderNames.push_back(input);
                    }
                    mode = Option::null;
                    stateEdits->push(nullptr);
                    break;
                }
            case Option::loadVertShaders:
                {
                    while (instream >> input)
                    {
                        auto cmd = loadVertPool.request(input);
                        cmdStack->push(std::move(cmd));
                        shaderReports.try_emplace(input, input, "Vertex", 0, 0, 0, 0);
                        shaderNames.push_back(input);
                    }
                    mode = Option::null;
                    stateEdits->push(nullptr);
                    break;
                }
            case Option::shaderReport:
                {
                    std::cout << "Should not get here" << std::endl;
                    break;
                }
            case Option::setSpecFloats:
                {
                    int first; int second; std::string type;
                    instream >> first >> second >> type;
                    assert (type == "f" || type == "v" && "Type specified incorrectly");
                    while (instream >> input)
                    {
                        auto cmd = ssfPool.request(input, type, first, second);
                        cmdStack->push(std::move(cmd));
                        auto& report = shaderReports.at(input);
                        report.setSpecFloat(0, first);
                        report.setSpecFloat(1, second);
                    }
                    mode = Option::null;
                    stateEdits->push(nullptr);
                    break;
                }
            case Option::setSpecInts:
                {
                    std::cout << "todo" << std::endl;
                    break;
                }
        }
        event->setHandled();
    }
    if (event->getCategory() == EventCategory::Abort)
    {
        mode = Option::null;
        stateEdits->reload(this);
    }
}

void State::RendererManager::handleEvent(Event* event, EditStack* stateEdits, CommandStack* cmdStack)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
        std::string input; 
        switch (mode)
        {
            case Mode::null:
                {
                    instream >> input;
                    Option option = opMap.findOption(input);
                    switch(option)
                    {
                        case Option::prepRenderFrames:
                            {
                                auto cmd = prfPool.request(&window);
                                cmdStack->push(std::move(cmd));
                                opMap.remove(Option::prepRenderFrames);
                                stateEdits->reload(this);
                                break;
                            }
                        case Option::createPipelineLayout:
                            {
                                auto cmd = cplPool.request();
                                cmdStack->push(std::move(cmd));
                                opMap.remove(Option::createPipelineLayout);
                                opMap.remove(Option::all);
                                stateEdits->reload(this);
                                break;
                            }
                        case Option::all:
                            {
                                auto ow = owPool.request();
                                cmdStack->push(std::move(ow));
                                auto pf = prfPool.request(&window);
                                cmdStack->push(std::move(pf));
                                auto cl = cplPool.request();
                                cmdStack->push(std::move(cl));
                                opMap.remove(Option::all);
                                opMap.remove(Option::createPipelineLayout);
                                opMap.remove(Option::openWindow);
                                opMap.add(Option::rpassManager);
                                opMap.add(Option::pipelineManager);
                                stateEdits->reload(this);
                                break;
                            }
                        case Option::rpassManager:
                            {
                                stateEdits->push(&rpassManager);
                                break;
                            }
                        case Option::pipelineManager:
                            {
                                stateEdits->push(&pipelineManager);
                                break;
                            }
                        case Option::openWindow:
                            {
                                auto cmd = owPool.request();
                                cmdStack->push(std::move(cmd));
                                opMap.remove(Option::openWindow);
                                opMap.remove(Option::all);
                                opMap.add(Option::prepRenderFrames);
                                break;
                            }
                        case Option::shaderManager:
                            {
                                stateEdits->push(&shaderManager);
                                break;
                            }
                        case Option::addattachState:
                            {
                                stateEdits->push(&addAttachState);
                                break;
                            }
                        case Option::null:
                            {
                                stateEdits->push(nullptr);
                                break;
                            }
                        case Option::printReports:
                            {
                                for (const auto& item : reports) 
                                    std::invoke(*item);   
                                break;
                            }
                        case Option::createRPI:
                            {
                                mode = Mode::createRPI;
                                break;
                            }
                    }
                    break;
                }
            case Mode::createRPI:
                {
                    std::string attachName;
                    std::string renderpassName;
                    std::string pipelineName;
                    instream >> attachName >> renderpassName >> pipelineName;
                    auto cmd = criPool.request(attachName, renderpassName, pipelineName);
                    if (cmd)
                        cmdStack->push(std::move(cmd));
                    mode = Mode::null;
                    break;
                }
        }
        event->setHandled();
    }
}

void State::RenderpassManager::handleEvent(Event* event, EditStack* stateEdits, CommandStack* cmdStack)
{    
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        switch (mode)
        {
            case Option::null:
                {
                    instream >> input;
                    Option option = opMap.findOption(input);
                    switch (option)
                    {
                        case Option::createSwapRenderpass:
                            {
                                std::cout << "Enter a name for the swapchain render pass" << std::endl;
                                mode = Option::createSwapRenderpass;
                                break;
                            }
                        case Option::null:
                            {
                                stateEdits->push(nullptr);
                                break;
                            }
                        case Option::report:
                            {
                                for (const auto& report : renderpassReports) 
                                {
                                    std::invoke(report);
                                }
                                break;
                            }
                    }
                    break;
                }
            case Option::createSwapRenderpass:
                {
                    instream >> input;
                    auto cmd = csrPool.request(input);
                    cmdStack->push(std::move(cmd));
                    renderpassReports.emplace_back(input);
                    stateEdits->push(nullptr);
                    mode = Option::null;
                    break;
                }
            case Option::report:
                {
                    std::cout << "Should not be here" << std::endl;
                    break;
                }
        }
        event->setHandled();
    }
}

void State::Paint::handleEvent(Event* event, EditStack* stateEdits, CommandStack* cmdStack)
{
    std::cout << "oooh" << std::endl;
}

void State::PipelineManager::handleEvent(Event* event, EditStack* stateEdits, CommandStack* cmdStack)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        switch (mode)
        {
            case Option::null:
            {
                instream >> input;
                Option option = opMap.findOption(input);
                switch (option)
                {
                    case Option::createGraphicsPipeline:
                    {
                        std::cout << "Enter a name for the pipeline, then provide names for"
                           " a pipelinelayout, a vertshader, a fragshader, "
                           " a renderpass, 4 numbers for the render region,"
                           " and a 1 or a 0 for whether or not this is a 3d or 2d pipeline" << std::endl;
                        mode = Option::createGraphicsPipeline;
                        stateEdits->push(this);
                        break;
                    }
                    case Option::report:
                    {
                        for (const auto& i : gpReports) 
                            std::invoke(i.second);
                        break;
                    }
                    case Option::null:
                    {
                        stateEdits->push(nullptr);
                        break;
                    }
                }
                break;
            }
            case Option::createGraphicsPipeline:
            {
                std::string name{"default"};
                std::string pipelineLayout{"default"};
                std::string vertshader{"default"};
                std::string fragshader{"default"};
                std::string renderpass{"default"};
                int a1;
                int a2;
                uint32_t a3;
                uint32_t a4;
                bool is3d{false};
                instream >> name >> pipelineLayout >> vertshader >> fragshader >> renderpass >> a1 >> a2 >> a3 >> a4 >> is3d;
                vk::Rect2D renderArea{{a1, a2}, {a3, a4}};
                auto cmd = cgpPool.request(name, pipelineLayout, vertshader, fragshader, renderpass, renderArea, is3d);
                if (cmd) 
                {
                    cmdStack->push(std::move(cmd));
                    gpReports.try_emplace(name, name, pipelineLayout, vertshader, fragshader, renderpass, a1, a2, a3, a4, is3d);
                }
                mode = Option::null;
                stateEdits->push(nullptr);
                break;
            }
            case Option::report:
            {
                std::cout << "hmmm" << std::endl;
                break;
            }
        }
    }
}

void State::Director::onEnter(Application* app) 
{
    std::vector<std::string> words;
    words.reserve(opMap.size());
    for (auto i : opMap) 
        words.push_back(i.first);
    app->setVocabulary(words);
}

void State::ShaderManager::onEnter(Application* app) 
{
    switch (mode)
    {
        case Option::null:
            {
                std::vector<std::string> words;
                words.reserve(opMap.size());
                for (auto i : opMap) 
                    words.push_back(i.first);
                app->setVocabulary(words);
                break;
            }
        case Option::loadFragShaders:
            {
                std::vector<std::string> vocab;
                auto dir = std::filesystem::directory_iterator(SHADER_DIR);
                for (const auto& entry : dir) 
                {
                    vocab.push_back(entry.path().filename());
                }
                app->setVocabulary(vocab);
                break;
            }
        case Option::loadVertShaders:
            {
                std::vector<std::string> vocab;
                auto dir = std::filesystem::directory_iterator(SHADER_DIR);
                for (const auto& entry : dir) 
                {
                    vocab.push_back(entry.path().filename());
                }
                app->setVocabulary(vocab);
                break;
            }
        case Option::shaderReport:
            break;
        case Option::setSpecFloats:
            {
                app->setVocabulary(shaderNames);
                break;
            }
        case Option::setSpecInts:
            {
                app->setVocabulary(shaderNames);
                break;
            }
    }
}

void State::AddAttachment::onEnter(Application* app) 
{
    std::vector<std::string> words;
    words.reserve(vocab.size());
    for (auto i : vocab) {
        words.push_back(i.first);
    }
    app->setVocabulary(words);
}

void State::Paint::onEnter(Application* app)
{
}

void State::PipelineManager::onEnter(Application* app)
{
    app->setVocabulary(opMap.getStrings());
}

void State::RendererManager::onEnter(Application* app)
{
    reports.clear();
    auto rpass = rpassManager.getReports();
    auto gp = pipelineManager.getReports();
    reports.insert(reports.begin(), rpass.begin(), rpass.end());
    reports.insert(reports.begin(), gp.begin(), gp.end());
    app->setVocabulary(opMap.getStrings());
}

void State::RenderpassManager::onEnter(Application* app)
{
    switch (mode)
    {
        case Option::null:
            {
                app->setVocabulary(opMap.getStrings());
                break;
            }
        case Option::createSwapRenderpass:
            {
                std::cout << "should not get here" << std::endl;
                break;
            }
        case Option::report:
            {
                break;
            }
    }
}

void Command::AddAttachment::execute(Application* app)
{
        vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
        if (isSampled)
            usage = usage | vk::ImageUsageFlagBits::eSampled;
        if (isTransferSrc)
            usage = usage | vk::ImageUsageFlagBits::eTransferSrc;
        app->renderer.createAttachment(attachmentName, dimensions, usage);
}

void Command::LoadFragShader::execute(Application* app)
{
    app->renderer.loadFragShader(SHADER_DIR + shaderName, shaderName);
}

void Command::LoadVertShader::execute(Application* app)
{
    app->renderer.loadVertShader(SHADER_DIR + shaderName, shaderName);
}

void Command::SetSpecFloat::execute(Application* app)
{
    if (type == "f")
    {
        FragShader& fs = app->renderer.fragShaderAt(shaderName);
        fs.setWindowResolution(x, y);
    }
    if (type == "v")
    {
        VertShader& vs = app->renderer.vertShaderAt(shaderName);
        vs.setWindowResolution(x, y);
    }
}

void Command::SetSpecInt::execute(Application* app)
{
    if (type == "f")
    {
        FragShader& fs = app->renderer.fragShaderAt(shaderName);
        fs.specData.integer0 = x;
        fs.specData.integer1 = y;
    }
    if (type == "v")
    {
        VertShader& vs = app->renderer.vertShaderAt(shaderName);
        vs.specData.integer0 = x;
        vs.specData.integer1 = y;
    }
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

void Command::CreateSwapchainRenderpass::execute(Application* app)
{
    auto& rpass = app->renderer.createRenderPass(rpassName);
    app->renderer.prepareAsSwapchainPass(rpass);
}

void Command::CreateGraphicsPipeline::execute(Application* app)
{
    auto& pipeline = app->renderer.createGraphicsPipeline(
            name, pipelineLayout,
            vertshader, fragshader,
            renderpass, renderArea, is3d);
    pipeline.create();
}

void Command::CreateRenderpassInstance::execute(Application* app)
{
    app->renderer.addRenderPassInstance(attachment, renderpass, pipeline);
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
            if (event)
            {
                if (recordevents) recordEvent(event, os);
                for (auto state : stateStack) 
                    if (!event->isHandled())
                        state->handleEvent(event, &stateEdits, &cmdStack);
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
            for (auto& cmd : cmdStack.items)
            {
                if (cmd)
                {
                    cmd->execute(this);
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

