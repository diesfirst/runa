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
    
constexpr const char* SHADER_DIR = "/home/michaelb/dev/sword/build/shaders/";

void State::State::pushState(State* state, EditStack* edits) {edits->pushState(state); activeChild = state;} 
void State::State::onResume(Application* app) {addDynamicOps(); onResumeExt(app); activeChild = nullptr;}
void State::State::onPush(Application* app) {removeDynamicOps(); onPushExt(app);}
void State::State::onExit(Application* app) {onExitExt(app); app->ev.popVocab();}
void State::State::onEnter(Application* app) {onEnterExt(app); app->ev.addVocab(&vocab);}
void State::State::refresh(Application* app) {app->ev.updateVocab();}

void State::AddAttachment::handleEvent(Event* event, EditStack* stateEdits, CommandStack* cmdStack)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        switch (mode)
        {
            case Mode::null:
                {
                    instream >> input;
                    Option option = opMap.findOption(input);
                    switch (option)
                    {
                        case Option::setdimensions:
                            {
                                std::cout << "Enter a width and height" << std::endl;
                                mode = Mode::setdimensions;
                                event->setHandled();
                                break;
                            }
                        case Option::fulldescription:
                            {
                                std::cout << "Enter a name, a width, a height, is sampled, is transfer src" << std::endl;
                                mode = Mode::fulldescription;
                                event->setHandled();
                                break;
                            }
                        case Option::addsamplesources:
                            {
                                std::cout << "enter names for attachments that will be sampled from" << std::endl;
                                mode = Mode::addsamplesources;
                                event->setHandled();
                                break;
                            }
                    }
                    break;
                }
            case Mode::setdimensions:
                {
                    instream >> width >> height;
                    mode = Mode::null;
                    event->setHandled();
                    break;
                };
            case Mode::addsamplesources:
                {
                    while (instream >> input)
                    {
                        auto cmd = addAttPool.request(input, width, height, true, false);
                        cmdStack->push(std::move(cmd));
                    }
                    mode = Mode::null;
                    event->setHandled();
                    break;
                }
            case Mode::fulldescription:
                {
                    std::string name; int w; int h; bool sample; bool transfersrc;
                    instream >> name >> w >> h >> sample >> transfersrc;
                    auto cmd = addAttPool.request(name, w, h, sample, transfersrc);
                    cmdStack->push(std::move(cmd));
                    mode = Mode::null;
                    event->setHandled();
                    break;
                }
        }
    }
    if (event->getCategory() == EventCategory::Abort && mode != Mode::null)
    {
        mode = Mode::null;
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
            case Mode::null:
                {
                    instream >> input;
                    Option option = opMap.findOption(input);
                    switch (option)
                    {
                        case Option::createSwapRenderpass:
                            {
                                std::cout << "Enter a name for the swapchain render pass" << std::endl;
                                mode = Mode::createSwapRenderpass;
                                break;
                            }
                        case Option::report:
                            {
                                for (const auto& report : renderpassReports) 
                                    std::invoke(*report);
                                break;
                            }
                    }
                    break;
                }
            case Mode::createSwapRenderpass:
                {
                    instream >> input;
                    auto cmd = csrPool.request(input);
                    cmdStack->push(std::move(cmd));
                    renderpassReports.emplace_back(std::make_unique<RenderpassReport>(input));
                    mode = Mode::null;
                    break;
                }
        }
        event->setHandled();
    }
    if (event->getCategory() == EventCategory::Abort && mode != Mode::null)
    {
        mode = Mode::null;
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
        auto conclude = [&]() mutable
        {
            auto refresh = refreshPool.request(this);
            cmdStack->push(std::move(refresh));
            event->setHandled();
        };
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        switch (mode)
        {
            case Mode::null:
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
                        mode = Mode::createGraphicsPipeline;
                        vocab = cgpVocab;
                        conclude();
                        break;
                    }
                    case Option::report:
                    {
                        for (const auto& i : gpReports) 
                            std::invoke(*i.second);
                        break;
                    }
                }
                break;
            }
            case Mode::createGraphicsPipeline:
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
                    gpReports.emplace(name, std::make_unique<GraphicsPipelineReport>(name, pipelineLayout, vertshader, fragshader, renderpass, a1, a2, a3, a4, is3d));
                }
                mode = Mode::null;
                vocab = opMap.getStrings();
                conclude();
                break;
            }
        }
    }
}

void State::Director::handleEvent(Event* event, EditStack* stateEdits, CommandStack* cmdStack)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
        std::string input;
        instream >> input;
        Option option = opMap.findOption(input);
        switch (option)
        {
            case Option::initRenState:
                {
                    pushState(&initRenState, stateEdits);
                    event->isHandled();
                    break;
                }
            case Option::printStack:
                {
                    std::cout << "====== State Stack ======" << std::endl;
                    stateStack.print();
                    event->isHandled();
                    break;
                }
            case Option::paint:
                {
                    pushState(&paint, stateEdits);
                    event->isHandled();
                    break;
                }
        }
    }
    if (event->getCategory() == EventCategory::Abort)
    {
        stateEdits->popState();
    }
}

void State::Director::onPushExt(Application* app)
{
    vocab = opMap.getStrings();
}

void State::Director::onResumeExt(Application* app)
{
    vocab = opMap.getStrings();
}

void State::RendererManager::handleEvent(Event* event, EditStack* stateEdits, CommandStack* cmdStack)
{
    auto conclude = [&]() mutable
    {
        vocab = opMap.getStrings();
        auto refresh = refreshPool.request(this);
        cmdStack->push(std::move(refresh));
        event->setHandled();
    };
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
                                conclude();
                                break;
                            }
                        case Option::createPipelineLayout:
                            {
                                auto cmd = cplPool.request();
                                cmdStack->push(std::move(cmd));
                                auto rs = refreshPool.request(this);
                                cmdStack->push(std::move(rs));
                                opMap.remove(Option::createPipelineLayout);
                                opMap.remove(Option::all);
                                conclude();
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
                                opMap.swapIn(Option::rpassManager);
                                opMap.swapIn(Option::pipelineManager);
                                opMap.swapIn(Option::createRPI);
                                opMap.swapIn(Option::render);
                                opMap.swapIn(Option::recordRenderCmd);
                                conclude();
                                break;
                            }
                        case Option::rpassManager:
                            {
                                pushState(&rpassManager, stateEdits);
                                event->setHandled();
                                break;
                            }
                        case Option::pipelineManager:
                            {
                                pushState(&pipelineManager, stateEdits);
                                event->setHandled();
                                break;
                            }
                        case Option::openWindow:
                            {
                                auto cmd = owPool.request();
                                cmdStack->push(std::move(cmd));
                                auto rs = refreshPool.request(this);
                                cmdStack->push(std::move(rs));
                                opMap.remove(Option::openWindow);
                                opMap.remove(Option::all);
                                opMap.swapIn(Option::prepRenderFrames);
                                conclude();
                                break;
                            }
                        case Option::shaderManager:
                            {
                                pushState(&shaderManager, stateEdits);
                                event->setHandled();
                                break;
                            }
                        case Option::addattachState:
                            {
                                pushState(&addAttachState, stateEdits);
                                event->setHandled();
                                break;
                            }
                        case Option::printReports:
                            {
                                for (const auto& item : reports) 
                                    std::invoke(*item);   
                                event->setHandled();
                                break;
                            }
                        case Option::createRPI:
                            {
                                mode = Mode::createRPI;
                                std::cout << "Enter an attachment name (or swap), a renderpass name, and a pipeline name" << std::endl;
                                vocab.clear(); 
                                vocab.push_back("swap");
                                for (const auto& i : reports) 
                                {
                                    auto type = i->getType();
                                    if (type == ReportType::Renderpass || type == ReportType::Pipeline)
                                        vocab.push_back(i->getObjectName());
                                }
                                auto cmd = refreshPool.request(this);
                                cmdStack->push(std::move(cmd));
                                event->setHandled();
                                break;
                            }
                        case Option::rpiReport:
                            {
                                for (const auto& item : rpiReports) 
                                    std::invoke(*item);
                                event->setHandled();
                                break;
                            }
                        case Option::render:
                            {
                                std::cout << "Enter a render command index and 0 or 1 based on whether or not to update ubo" << std::endl;
                                mode = Mode::render;
                                event->setHandled();
                                break;
                            }
                        case Option::recordRenderCmd:
                            {
                                std::cout << "Enter a command index, and an array of render pass instance indices to use." << std::endl;
                                mode = Mode::recordRenderCmd;
                                event->setHandled();
                                break;
                            }
                    }
                    break;
                }
            case Mode::createRPI:
                {
                    auto find = [this](const std::string& s) mutable
                    {
                        for (const auto& i : vocab) 
                            if (s == i)
                             return true;   
                        return false;
                    };
                    std::string attachName;
                    std::string renderpassName;
                    std::string pipelineName;
                    instream >> attachName >> renderpassName >> pipelineName;
                    if (!find(attachName) || !find(renderpassName) || !find(pipelineName))
                    {
                        std::cout << "Bad input" << std::endl;
                        break;
                    }
                    auto cmd = criPool.request(attachName, renderpassName, pipelineName);
                    cmdStack->push(std::move(cmd));
                    auto report = std::make_unique<RenderpassInstanceReport>(attachName, renderpassName, pipelineName, rpiReports.size());
                    const auto& r = rpiReports.emplace_back(std::move(report));
                    reports.push_back(r.get());
                    mode = Mode::null;
                    break;
                }
            case Mode::render:
                {
                    int rendercmdIndex;
                    bool updateUbo;
                    instream >> rendercmdIndex >> updateUbo;
                    auto cmd = renderPool.request(rendercmdIndex, updateUbo);
                    cmdStack->push(std::move(cmd));
                    mode = Mode::null;
                    break;
                }
            case Mode::recordRenderCmd:
                {
                    int cmdIndex;
                    uint32_t i;
                    std::vector<uint32_t> rpiIndices;
                    instream >> cmdIndex;
                    while (instream >> i)
                        rpiIndices.push_back(i);
                    auto cmd = rrcPool.request(cmdIndex, rpiIndices);
                    cmdStack->push(std::move(cmd));
                    auto& r = rcReports.emplace_back(std::make_unique<RenderCommandReport>(cmdIndex, rpiIndices));
                    reports.push_back(r.get());
                    mode = Mode::null;
                    break;
                }
        }
    }
    if (event->getCategory() == EventCategory::Abort)
    {
        if (mode != Mode::null)
        {
            mode = Mode::null;
            conclude();
        }
    }
}

//TODO: we can't store pointers to vector elements
//      we should instead change over the report collection
//      mechanism to store pointers to the vectors of reports
//      themselves. const pointers.
void State::RendererManager::onResumeExt(Application* app)
{
    auto syncReports = [this](const auto& t)
    {
        if (activeChild == &t)
        {
            auto newReports = t.getReports();
            for (const auto& nr : newReports) 
            {
                bool alreadyIn{false};
                for (auto& r : reports)
                    if (r->getObjectName() == nr->getObjectName()) 
                    {
                        alreadyIn = true;
                        r = nr;
                        break;
                    }
                if (!alreadyIn)
                    reports.push_back(nr);   
            }
        }
    };
    syncReports(shaderManager);
    syncReports(rpassManager);
    syncReports(pipelineManager);
}

void State::RendererManager::onPushExt(Application* app)
{
    if (activeChild == &pipelineManager)
    {
        std::vector<std::string> possibleCPArgs;
        for (const auto& i : reports) 
        {
            auto type = i->getType();
            if (type  == rpt::ReportType::Renderpass || type == rpt::ReportType::Shader)
            {
                auto name = i->getObjectName();   
                possibleCPArgs.push_back(name);
            }
        }
        pipelineManager.setCGPVocab(possibleCPArgs);
    }
}

void State::ShaderManager::setShaderVocab()
{
    vocab.clear();
    auto dir = std::filesystem::directory_iterator(SHADER_DIR);
    for (const auto& entry : dir) 
        vocab.push_back(entry.path().filename());
}

void State::ShaderManager::handleEvent(Event* event, EditStack* stateEdits, CommandStack* cmdStack)
{
    auto conclude = [&]() mutable 
    {
        auto rs = refreshPool.request(this);
        cmdStack->push(std::move(rs));
        event->setHandled();
    };
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getInput();
        std::stringstream instream{input};
        switch (mode)
        {
            case Mode::null:
                {
                    instream >> input;
                    Option option = opMap.findOption(input);
                    switch (option)
                    {
                        case Option::loadFragShaders:
                            {
                                mode = Mode::loadFragShaders;
                                std::cout << "Choose which frag shaders to load." << std::endl;
                                setShaderVocab();
                                conclude();
                                break;
                            }
                        case Option::loadVertShaders:
                            {
                                mode = Mode::loadVertShaders;
                                std::cout << "Choose which vert shaders to load." << std::endl;
                                setShaderVocab();
                                conclude();
                                break;
                            }
                        case Option::shaderReport:
                            {
                                for (const auto& item : shaderReports) 
                                    std::invoke(*item.second);
                                event->setHandled();
                                break;
                            }
                        case Option::setSpecFloats:
                            {
                                mode = Mode::setSpecFloats;
                                std::cout << "Enter two floats, v or f, and the names of the shaders to set" << std::endl;
                                setShaderVocab();
                                vocab.clear();
                                for (const auto& i : shaderReports) 
                                {
                                    vocab.push_back(i.second->getObjectName());
                                }
                                conclude();
                                break;
                            }
                        case Option::setSpecInts:
                            {
                                mode = Mode::setSpecInts;
                                std::cout << "Enter two ints, v or f, and the names of the shaders to set" << std::endl;
                                setShaderVocab();
                                conclude();
                                break;
                            }
                    }
                    break;
                }
            case Mode::loadFragShaders:
                {
                    while (instream >> input)
                    {
                        auto cmd = loadFragPool.request(input);
                        cmdStack->push(std::move(cmd));
                        shaderReports.emplace(input, std::make_unique<ShaderReport>(input, "Vertex", 0, 0, 0, 0));
                    }
                    mode = Mode::null;
                    vocab = opMap.getStrings();
                    conclude();
                    break;
                }
            case Mode::loadVertShaders:
                {
                    while (instream >> input)
                    {
                        auto cmd = loadVertPool.request(input);
                        cmdStack->push(std::move(cmd));
                        shaderReports.emplace(input, std::make_unique<ShaderReport>(input, "Vertex", 0, 0, 0, 0));
                    }
                    mode = Mode::null;
                    vocab = opMap.getStrings();
                    conclude();
                    break;
                }
            case Mode::setSpecFloats:
                {
                    int first; int second; std::string type;
                    instream >> first >> second >> type;
                    assert (type == "f" || type == "v" && "Type specified incorrectly");
                    while (instream >> input)
                    {
                        auto cmd = ssfPool.request(input, type, first, second);
                        cmdStack->push(std::move(cmd));
                        auto& report = shaderReports.at(input);
                        report->setSpecFloat(0, first);
                        report->setSpecFloat(1, second);
                    }
                    mode = Mode::null;
                    vocab = opMap.getStrings();
                    conclude();
                    break;
                }
            case Mode::setSpecInts:
                {
                    std::cout << "todo" << std::endl;
                    mode = Mode::null;
                    vocab = opMap.getStrings();
                    conclude();
                    break;
                }
        }
    }
    if (event->getCategory() == EventCategory::Abort)
    {
        if (mode != Mode::null)
        {
            mode = Mode::null;
            vocab = opMap.getStrings();
            conclude();
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

void Command::RecordRenderCommand::execute(Application* app)
{
    app->renderer.recordRenderCommands(cmdBufferId, renderpassInstances);
}

void Command::Render::execute(Application* app)
{
    app->renderer.render(renderCommandId, updateUBO);
}

void Command::RefreshState::execute(Application* app)
{
    state->refresh(app);
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
    dirState{window, stateStack}
{
    stateStack.push(&dirState);
    dirState.onEnter(this);
    ev.updateVocab();
}

void Application::popState()
{
    stateStack.top()->onExit(this);
    stateStack.pop();
    auto top = stateStack.top();
    top->onResume(this);
    ev.updateVocab();
}

void Application::pushState(State::State* state)
{
    stateStack.top()->onPush(this);
    stateStack.push(std::move(state));
    state->onEnter(this);
    ev.updateVocab();
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
    if (event->getCategory() == EventCategory::Abort)
    {

        os.open(eventlog, std::ios::out | std::ios::binary | std::ios::app);
        auto ce = static_cast<Abort*>(event);
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
        EventCategory ec = Event::unserializeCategory(is);
        if (ec == EventCategory::CommandLine)
        {
            auto cmdevnt = std::make_unique<CommandLineEvent>("foo");
            cmdevnt->unserialize(is);
            ev.eventQueue.push(std::move(cmdevnt));
            std::cout << "pushed commandline event" << std::endl;
        }
        if (ec == EventCategory::Abort)
        {
            auto abrt = std::make_unique<Abort>();
            ev.eventQueue.push(std::move(abrt));
            std::cout << "pushed abort event" << std::endl;
        }
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
            for (auto state : stateEdits) 
            {
                if (state)
                    pushState(state);
                else
                    popState();
            }
            stateEdits.clear();
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

