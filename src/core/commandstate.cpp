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
        std::cout << ENDL;\
        app->setVocabulary(vocab)\

#define ENDL std::endl;

constexpr const char* SHADER_DIR = "/home/michaelb/dev/sword/build/shaders/";

namespace State
{

void State::pushState(State* state) {stateEdits.pushState(state); activeChild = state;} 
void State::pushCmd(CmdPtr cmd) {cmdStack.push(std::move(cmd));}
void State::onResume(Application* app) {addDynamicOps(); syncReports(activeChild); onResumeExt(app); activeChild = nullptr;}
void State::onPush(Application* app) {removeDynamicOps(); onPushExt(app);}
void State::onExit(Application* app) {onExitExt(app); app->ev.popVocab();}
void State::onEnter(Application* app) 
{
    onEnterExt(app); app->ev.addVocab(vocab.getValues());
    std::cout << "Commandline options:   "; 
    for (const auto& i : vocab) 
    {
        std::cout << i << "   ";
    }
    std::cout << std::endl;
}
void State::refresh(Application* app) {app->ev.updateVocab();}
void State::syncReports(State* t)
{
    auto newReports = t->getReports();
    for (const auto& nr : newReports) 
    {
        bool alreadyIn{false};
        for (auto& r : reports)
            if (r->getObjectName() == nr->getObjectName() && r->getType() == nr->getType()) 
            {
                alreadyIn = true;
                r = nr;
                break;
            }
        if (!alreadyIn)
            reports.push_back(nr);   
    }
}

void DescriptorManager::base(Event* event)
{
    auto input = static_cast<CommandLineEvent*>(event)->getFirstWord();
    auto option = opMap.findOption(input);
    if (!option) return;
    switch (*option)
    {
        case Option::createDescriptorSetLayout:
        {
            pushState(&cdsl);
            event->setHandled();
            break;
        }
        case Option::createFrameDescriptorSets:
        {
            std::cout << "Enter the descriptor set layout names" << std::endl;
            mode = Mode::createFrameDescriptorSets;
            event->setHandled();
            break;
        }
        case Option::printReports:
        {
            for (const auto& i : reports) 
            {
                std::invoke(*i);
                event->setHandled();
            }
            break;
        }
        case Option::initFrameUbos:
        {
            std::cout << "Enter a binding number" << std::endl;
            mode = Mode::initFrameUbos;
            event->setHandled();
            break;
        }
        case Option::updateFrameSamplers:
        {
            std::cout << "Enter a binding number" << std::endl;           
            mode = Mode::updateFrameSamplers;
            event->setHandled();
            break;
        }
    }
}

void DescriptorManager::createFrameDescriptorSets(Event* event)
{
    std::stringstream ss{static_cast<CommandLineEvent*>(event)->getInput()};
    std::vector<std::string> layoutNames;
    std::string name;
    while (ss >> name)
        layoutNames.push_back(name);
    auto cmd = cfdsPool.request(layoutNames);
    cmdStack.push(std::move(cmd));
    descSetReports.emplace_back(layoutNames, rpt::DescriptorSetReport::Type::frameOwned);
    mode = Mode::null;
    event->setHandled();
}

void DescriptorManager::initFrameUbos(Event* event)
{
    auto ss = static_cast<CommandLineEvent*>(event)->getStream();
    int binding; 
    ss >> binding;
    auto cmd = ifuPool.request(binding);
    pushCmd(std::move(cmd));
    mode = Mode::null;
    event->setHandled();
}

void DescriptorManager::updateFrameSamplers(Event* event)
{
    auto ss = static_cast<CommandLineEvent*>(event)->getStream();
    int binding; 
    ss >> binding;
    auto cmd = ufsPool.request(binding);
    pushCmd(std::move(cmd));
    mode = Mode::null;
    event->setHandled();
}

void DescriptorManager::handleEvent(Event* event)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        MemFunc func = *funcMap.findOption(mode);
        std::invoke(func, this, event);
    }
}

void CreateDescriptorSetLayout::enterBindings(Event* event)
{
    auto cmdEvent = static_cast<CommandLineEvent*>(event);
    switch(static_cast<Stage>(curStage))
    {
        case Stage::descriptorTypeEntry:
        {
            auto option = descTypeMap.findOption(cmdEvent);
            if (!option) return;
            pendingBindings.at(bindingCount).setDescriptorType(*option);
            curStage++;
            vocab = {};
            event->setHandled();
            std::cout << "Enter descriptor count" << std::endl;
            break;
        }
        case Stage::descriptorCountEntry:
        {
            auto input = cmdEvent->getInput();
            std::stringstream ss{input};
            int count;
            ss >> count;
            pendingBindings.at(bindingCount).setDescriptorCount(count);
            curStage++;
            std::cout << "Enter shader stage" << std::endl;
            vocab = shaderStageMap.getStrings();
            event->setHandled();
            break;
        }
        case Stage::shaderStageEntry:
        {
            auto option = shaderStageMap.findOption(cmdEvent);
            if (!option) return;
            pendingBindings.at(bindingCount).setStageFlags(*option);
            bindingCount++;
            mode = Mode::null;
            vocab = opMap.getStrings();
            event->setHandled();
            break;
        }
    }
}

void CreateDescriptorSetLayout::initial(Event* event)
{
    auto cmdEvent = static_cast<CommandLineEvent*>(event);
    auto option = opMap.findOption(cmdEvent);
    if (!option) return;
    switch (*option)
    {
        case Option::enterBindings:
        {
            std::cout << "Enter a state type." << std::endl;
            vocab = descTypeMap.getStrings();
            pendingBindings.resize(bindingCount + 1);
            pendingBindings.at(bindingCount).setBinding(bindingCount);
            curStage = 0;
            mode = Mode::enterBindings;
            event->setHandled();
            break;
        }
        case Option::createDescriptorSetLayout:
        {
            if (pendingBindings.size() < 1)
            {
                std::cout << "Not enough bindings" << std::endl;
                event->setHandled();
                break;
            }
            std::cout << "Enter a name for the layout" << std::endl;
            vocab = {};
            mode = Mode::createLayout;
            event->setHandled();
            break;
        }
    }
}

void CreateDescriptorSetLayout::createLayout(Event* event)
{
    auto cmdEvent = static_cast<CommandLineEvent*>(event);
    std::stringstream ss{cmdEvent->getInput()};
    std::string name;
    ss >> name;
    auto cmd = cdslPool.request(name, pendingBindings);
    cmdStack.push(std::move(cmd));
    descSetLayoutReports.emplace_back(name, pendingBindings);
    pendingBindings.clear();
    vocab = opMap.getStrings();
    event->setHandled();
}

void CreateDescriptorSetLayout::handleEvent(Event* event)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto option = funcMap.findOption(mode);
        if (!option) return;
        std::invoke(*option, this, event);
    }
    if (event->getCategory() == EventCategory::Abort)
    {
        if (mode == Mode::enterBindings)
        {
            mode = Mode::null;
            vocab = opMap.getStrings();
            pendingBindings.pop_back();
            event->setHandled();
        }
    }
}

void AddAttachment::handleEvent(Event* event)
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
                    auto option = opMap.findOption(input);
                    if (!option) return;
                    switch (*option)
                    {
                        case Option::setdimensions:
                            {
                                std::cout << "Enter a width and height" << ENDL;
                                mode = Mode::setdimensions;
                                event->setHandled();
                                break;
                            }
                        case Option::fulldescription:
                            {
                                std::cout << "Enter a name, a width, a height, is sampled, is transfer src" << ENDL;
                                mode = Mode::fulldescription;
                                event->setHandled();
                                break;
                            }
                        case Option::addsamplesources:
                            {
                                std::cout << "enter names for attachments that will be sampled from" << ENDL;
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
                        vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment | 
                            vk::ImageUsageFlagBits::eSampled;
                        auto cmd = addAttPool.request(input, width, height, usage);
                        cmdStack.push(std::move(cmd));
                    }
                    mode = Mode::null;
                    event->setHandled();
                    break;
                }
            case Mode::fulldescription:
                {
                    std::string name; int w; int h; bool sample; bool transfersrc;
                    instream >> name >> w >> h >> sample >> transfersrc;
                    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
                    if (sample)
                        usage = usage | vk::ImageUsageFlagBits::eSampled;
                    if (transfersrc)
                        usage = usage | vk::ImageUsageFlagBits::eTransferSrc;
                    auto cmd = addAttPool.request(name, w, h, usage);
                    cmdStack.push(std::move(cmd));
                    attachmentReports.emplace_back(name, w, h, usage);
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

RenderpassManager::ReturnType RenderpassManager::initial(Event* event)
{
    auto input = static_cast<CommandLineEvent*>(event)->getFirstWord();
    auto option = opMap.findOption(input);
    if (option) 
        switch (*option)
        {
            case Option::createSwapRenderpass:
                {
                    std::cout << "Enter a name for the swapchain render pass" << std::endl;
                    return Mode::createSwapRenderpass;
                }
            case Option::createOffscreenRenderpass:
                {
                    std::cout << "Enter a name for the offscreen render pass and load or clear to define the attachment load op" << std::endl;   
                    return Mode::createOffscreenRenderpass;
                }
            case Option::report:
                {
                    for (const auto& report : renderpassReports) 
                        std::invoke(report);
                    return Mode::null;
                }
        }
    return {};
}

RenderpassManager::ReturnType RenderpassManager::createSwapRenderpass(Event* event)
{
    auto instream = static_cast<CommandLineEvent*>(event)->getStream();
    std::string name;
    instream >> name;
    auto cmd = csrPool.request(name);
    if (cmd)
    {
        pushCmd(std::move(cmd));
        renderpassReports.emplace_back(name, rpt::RenderpassReport::Type::swapchain);
        return mode = Mode::null;
    }
    return {};
}

RenderpassManager::ReturnType RenderpassManager::createOffscreenRenderpass(Event* event)
{
    auto instream = static_cast<CommandLineEvent*>(event)->getStream();
    std::string name;
    std::string opString;
    vk::AttachmentLoadOp loadOp;
    instream >> name;
    instream >> opString;
    if (opString == "load")
        loadOp = vk::AttachmentLoadOp::eLoad;
    else if (opString == "clear")
        loadOp = vk::AttachmentLoadOp::eClear;
    else
    {
        std::cout << "Bad load op" << std::endl;
        return {};
    }
    auto cmd = corPool.request(name, loadOp);
    if (cmd)
    {
        pushCmd(std::move(cmd));
        renderpassReports.emplace_back(
                name, rpt::RenderpassReport::Type::offscreen, loadOp);
        return mode = Mode::null;
    }
    return {};
}

void RenderpassManager::handleEvent(Event* event)
{    
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto option = modeToFunc.findOption(mode);
        if (!option) return;
        auto res = std::invoke(*option, this, event);
        if (res)
        {
            mode = *res;
            event->setHandled();
            std::cout << "Success" << std::endl;
        }
    }
    if (event->getCategory() == EventCategory::Abort && mode != Mode::null)
    {
        mode = Mode::null;
    }
}

void PipelineManager::base(Event* event)
{
    auto input = static_cast<CommandLineEvent*>(event)->getFirstWord();
    auto option = opMap.findOption(input);
    if (!option) return;
    switch (*option)
    {
        case Option::createGraphicsPipeline:
        {
            std::cout << "Enter a name for the pipeline, then provide names for"
               " a pipelinelayout, a vertshader, a fragshader, "
               " a renderpass, 4 numbers for the render region,"
               " and a 1 or a 0 for whether or not this is a 3d or 2d pipeline" << ENDL;
            mode = Mode::createGraphicsPipeline;
            vocab = cgpVocab;
            event->setHandled();
            break;
        }
        case Option::report:
        {
            for (const auto& i : gpReports) 
                std::invoke(*i.second);
            break;
        }
        case Option::createPipelineLayout:
        {
            std::cout << "Enter name a name for the pipeline layout and the names of the descriptor set layouts" << std::endl;
            mode = Mode::createPipelineLayout;
            break;
        }
    }
}

void PipelineManager::createPipelineLayout(Event* event)
{
    auto instream = static_cast<CommandLineEvent*>(event)->getStream();
    std::string name;
    std::string layout;
    std::vector<std::string> layoutnames;
    instream >> name;
    while (instream >> layout)
        layoutnames.push_back(layout);
    auto cmd = cpplPool.request(name, layoutnames);
    cmdStack.push(std::move(cmd));
    pipelineLayoutReports.emplace_back(name, layoutnames);
    event->setHandled();
    mode = Mode::null;
}

void PipelineManager::createGraphicsPipeline(Event* event)
{
    auto instream = static_cast<CommandLineEvent*>(event)->getStream();
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
        cmdStack.push(std::move(cmd));
        gpReports.emplace(name, std::make_unique<GraphicsPipelineReport>(name, pipelineLayout, vertshader, fragshader, renderpass, a1, a2, a3, a4, is3d));
    }
    mode = Mode::null;
    vocab = opMap.getStrings();
    event->setHandled();
}

void PipelineManager::handleEvent(Event* event)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto option = modeToFunc.findOption(mode);
        if (option)
            std::invoke(*option, this, event);
    }
    if (event->getCategory() == EventCategory::Abort && mode != Mode::null)
    {
        mode = Mode::null;
        vocab = opMap.getStrings();
    }
}

void Director::handleEvent(Event* event)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
        std::string input;
        instream >> input;
        auto option = opMap.findOption(input);
        if (!option) return;
        switch (*option)
        {
            case Option::initRenState:
                {
                    pushState(&initRenState);
                    event->isHandled();
                    break;
                }
            case Option::printStack:
                {
                    std::cout << "====== State Stack ======" << ENDL;
                    stateStack.print();
                    event->isHandled();
                    break;
                }
            case Option::paint:
                {
                    pushState(&paint);
                    event->isHandled();
                    break;
                }
        }
    }
    if (event->getCategory() == EventCategory::Abort)
    {
        if (stateStack.size() > 1)
            stateEdits.popState();
    }
}

void Director::onPushExt(Application* app)
{
    vocab = opMap.getStrings();
}

void Director::onResumeExt(Application* app)
{
    vocab = opMap.getStrings();
}

void RendererManager::base(Event* event)
{
    std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
    std::string input;
    instream >> input;
    auto option = opMap.findOption(input);
    if (!option) return;
    switch(*option)
    {
        case Option::prepRenderFrames:
            {
                auto cmd = prfPool.request(&window);
                cmdStack.push(std::move(cmd));
                opMap.remove(Option::prepRenderFrames);
                dependentMap.move(Option::descriptionManager, opMap);
                dependentMap.move(Option::rpassManager, opMap);
                dependentMap.move(Option::pipelineManager, opMap);
                dependentMap.move(Option::createRPI, opMap);
                dependentMap.move(Option::render, opMap);
                dependentMap.move(Option::recordRenderCmd, opMap);
                vocab = opMap.getStrings();
                event->setHandled();
                break;
            }
        case Option::all:
            {
                auto ow = owPool.request();
                cmdStack.push(std::move(ow));
                auto pf = prfPool.request(&window);
                cmdStack.push(std::move(pf));
                opMap.remove(Option::all);
                opMap.remove(Option::openWindow);
                dependentMap.move(Option::rpassManager, opMap);
                dependentMap.move(Option::pipelineManager, opMap);
                dependentMap.move(Option::createRPI, opMap);
                dependentMap.move(Option::render, opMap);
                dependentMap.move(Option::recordRenderCmd, opMap);
                vocab = opMap.getStrings();
                event->setHandled();
                break;
            }
        case Option::rpassManager:
            {
                pushState(&rpassManager);
                event->setHandled();
                break;
            }
        case Option::pipelineManager:
            {
                pushState(&pipelineManager);
                event->setHandled();
                break;
            }
        case Option::openWindow:
            {
                auto cmd = owPool.request();
                cmdStack.push(std::move(cmd));
                auto rs = refreshPool.request(this);
                cmdStack.push(std::move(rs));
                opMap.remove(Option::openWindow);
                opMap.remove(Option::all);
                dependentMap.move(Option::prepRenderFrames, opMap);
                vocab = opMap.getStrings();
                event->setHandled();
                break;
            }
        case Option::shaderManager:
            {
                pushState(&shaderManager);
                event->setHandled();
                break;
            }
        case Option::addattachState:
            {
                pushState(&addAttachState);
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
                std::cout << "Enter an attachment name (or swap), a renderpass name, and a pipeline name" << ENDL;
                std::vector<std::string> words;
                words.push_back("swap");
                for (const auto& i : reports) 
                {
                    auto type = i->getType();
                    if (type == ReportType::Renderpass || type == ReportType::Pipeline)
                        words.push_back(i->getObjectName());
                }
                vocab = words;
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
                std::cout << "Enter a render command index and 0 or 1 based on whether or not to update ubo" << ENDL;
                mode = Mode::render;
                event->setHandled();
                break;
            }
        case Option::recordRenderCmd:
            {
                std::cout << "Enter a command index, and an array of render pass instance indices to use." << ENDL;
                mode = Mode::recordRenderCmd;
                event->setHandled();
                break;
            }
        case Option::descriptionManager:
            {
                pushState(&descriptorManager);
                event->setHandled();
                break;
            }
    }
}

void RendererManager::createRPI(Event* event)
{
    std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
    auto find = [this](const std::string& s) mutable
    {
        for (const auto& i : vocab) 
            if (s == i)
             return true;   
        return false;
    };
    std::string attachName, renderpassName, pipelineName;
    instream >> attachName >> renderpassName >> pipelineName;
    if (!find(attachName) || !find(renderpassName) || !find(pipelineName))
        std::cout << "Bad input" << ENDL;
    auto cmd = criPool.request(attachName, renderpassName, pipelineName);
    cmdStack.push(std::move(cmd));
    auto report = std::make_unique<RenderpassInstanceReport>(attachName, renderpassName, pipelineName, rpiReports.size());
    const auto& r = rpiReports.emplace_back(std::move(report));
    reports.push_back(r.get());
    mode = Mode::null;
    vocab = opMap.getStrings();
    event->setHandled();
}

void RendererManager::recordRenderCmd(Event* event)
{
    std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
    int cmdIndex;
    uint32_t i;
    std::vector<uint32_t> rpiIndices;
    instream >> cmdIndex;
    while (instream >> i)
        rpiIndices.push_back(i);
    auto cmd = rrcPool.request(cmdIndex, rpiIndices);
    cmdStack.push(std::move(cmd));
    auto& r = rcReports.emplace_back(std::make_unique<RenderCommandReport>(cmdIndex, rpiIndices));
    reports.push_back(r.get());
    mode = Mode::null;
    vocab = opMap.getStrings();
    event->setHandled();
}

void RendererManager::render(Event* event)
{
    std::stringstream instream{static_cast<CommandLineEvent*>(event)->getInput()};
    int rendercmdIndex;
    bool updateUbo;
    instream >> rendercmdIndex >> updateUbo;
    auto cmd = renderPool.request(rendercmdIndex, updateUbo);
    cmdStack.push(std::move(cmd));
    mode = Mode::null;
    vocab = opMap.getStrings();
    event->setHandled();
}

void RendererManager::handleEvent(Event* event)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto option = modeToFunc.findOption(mode);
        if (!option) return;
        std::invoke(*option, this, event);
    }
    if (event->getCategory() == EventCategory::Abort)
    {
        if (mode != Mode::null)
        {
            mode = Mode::null;
            vocab = opMap.getStrings();
            event->setHandled();
        }
    }
}

void RendererManager::onPushExt(Application* app)
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

void ShaderManager::setShaderVocab()
{
    std::vector<std::string> words;
    auto dir = std::filesystem::directory_iterator(SHADER_DIR);
    for (const auto& entry : dir) 
        words.push_back(entry.path().filename());
    vocab = words;
}

void ShaderManager::handleEvent(Event* event)
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
                    auto option = opMap.findOption(input);
                    if (!option) return;
                    switch (*option)
                    {
                        case Option::loadFragShaders:
                            {
                                mode = Mode::loadFragShaders;
                                std::cout << "Choose which frag shaders to load." << ENDL;
                                setShaderVocab();
                                event->setHandled();
                                break;
                            }
                        case Option::loadVertShaders:
                            {
                                mode = Mode::loadVertShaders;
                                std::cout << "Choose which vert shaders to load." << ENDL;
                                setShaderVocab();
                                event->setHandled();
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
                                std::cout << "Enter two floats, v or f, and the names of the shaders to set" << ENDL;
                                setShaderVocab();
                                event->setHandled();
                                break;
                            }
                        case Option::setSpecInts:
                            {
                                mode = Mode::setSpecInts;
                                std::cout << "Enter two ints, v or f, and the names of the shaders to set" << ENDL;
                                setShaderVocab();
                                event->setHandled();
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
                        cmdStack.push(std::move(cmd));
                        shaderReports.emplace(input, std::make_unique<ShaderReport>(input, "Vertex", 0, 0, 0, 0));
                    }
                    mode = Mode::null;
                    vocab = opMap.getStrings();
                    event->setHandled();
                    break;
                }
            case Mode::loadVertShaders:
                {
                    while (instream >> input)
                    {
                        auto cmd = loadVertPool.request(input);
                        cmdStack.push(std::move(cmd));
                        shaderReports.emplace(input, std::make_unique<ShaderReport>(input, "Vertex", 0, 0, 0, 0));
                    }
                    mode = Mode::null;
                    vocab = opMap.getStrings();
                    event->setHandled();
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
                        cmdStack.push(std::move(cmd));
                        auto& report = shaderReports.at(input);
                        report->setSpecFloat(0, first);
                        report->setSpecFloat(1, second);
                    }
                    mode = Mode::null;
                    vocab = opMap.getStrings();
                    event->setHandled();
                    break;
                }
            case Mode::setSpecInts:
                {
                    int first; int second; std::string type;
                    instream >> first >> second >> type;
                    assert (type == "f" || type == "v" && "Type specified incorrectly");
                    while (instream >> input)
                    {
                        auto cmd = ssiPool.request(input, type, first, second);
                        cmdStack.push(std::move(cmd));
                        auto& report = shaderReports.at(input);
                        report->setSpecInt(0, first);
                        report->setSpecInt(1, second);
                    }
                    mode = Mode::null;
                    vocab = opMap.getStrings();
                    event->setHandled();
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
            event->setHandled();
        }
    }
}

Paint::ReturnType Paint::initial(Event* event)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto input = static_cast<CommandLineEvent*>(event)->getFirstWord();
        auto option = opMap.findOption(input);
        if (!option) return;
        switch (*option)
        {
            case Option::setup:
            {
                std::cout << "Enter the render command id to use for painting." << std::endl;
                mode = Mode::setup;
                event->setHandled();
                vocab = {};
                break;
            }
            case Option::paint:
            {
                mode = Mode::paint;
                event->setHandled();
                vocab = {};
                break;
            }
        }
    }
}

Paint::ReturnType Paint::setup(Event* event)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {
        auto ss = static_cast<CommandLineEvent*>(event)->getStream();
        ss >> paintCommand;
        std::cout << "Paint command set to: " << paintCommand << std::endl;
        mode = Mode::initial;
        vocab = opMap.getStrings();
    }
}

Paint::ReturnType Paint::paint(Event* event)
{
    if (event->getCategory() == EventCategory::Window)
    {
        auto winevent = static_cast<WindowEvent*>(event);
        if (winevent->getType() == WindowEventType::Motion)
        {
            auto input = static_cast<MouseMotionEvent*>(event);
//            mPos = glm::vec4{
//                input->getX() / float(width), input->getY() / float(width), 1., 1.};
//            mPos = fragInput.xform * mPos;
            fragInput.mouseX = input->getX() / float(width);
            fragInput.mouseY = input->getY() / float(height);
            std::cout << "X: " << mPos.x << " Y: " << mPos.y << std::endl;
            auto cmd = renderPool.request(paintCommand, true);
            pushCmd(std::move(cmd));
        }
    }
}

Paint::ReturnType Paint::paint_hm(Event* event)
{
    if (event->getCategory() == EventCategory::Window)
    {
        auto winevent = static_cast<WindowEvent*>(event);
        switch(winevent->getType())
        {
            case WindowEventType::Motion:
                {
                    std::cout << "got motion" << std::endl;
                    auto input = static_cast<MouseMotionEvent*>(winevent);
                    glm::vec4 mPos = glm::vec4{
                        input->getX() / float(width), input->getY() / float(width), 1., 1.};
                    switch (state)
                    {
                        case InputState::Paint:
                            {
                                mPos = fragInput.xform * mPos;
                                fragInput.mouseX = mPos.x;
                                fragInput.mouseY = mPos.y;
                                auto cmd = renderPool.request(paintCommand, true);
                                pushCmd(std::move(cmd));
                                break;
                            }
                        case InputState::Idle:
                            {
                                mPos = fragInput.xform * mPos;
                                fragInput.mouseX = mPos.x;
                                fragInput.mouseY = mPos.y;
                                auto cmd = renderPool.request(brushSizeCmd, true);
                                pushCmd(std::move(cmd));
                                break;
                            }
                        case InputState::Rotate:
                            {
                                angle = 
                                    angleScale * input->getX() / float(width) - mouseXCache + 
                                    angleScale * input->getY()/ float(height) - mouseYCache;
                                scaleRotMatrix = scaleRotCache * transMatrix * glm::rotate(glm::mat4(1.), angle, {0, 0, 1.}) * glm::inverse(transMatrix);
                                fragInput.xform = 
                                    glm::inverse(pivotMatrix) * 
                                    scaleRotMatrix * 
                                    transMatrix * 
                                    pivotMatrix * 
                                    toCanvasSpace;
                                auto cmd = renderPool.request(swapOnlyCmd, true);
                                pushCmd(std::move(cmd));
                                break;
                            }
                        case InputState::Scale:
                            {
                                scale = 
                                    mPos.x - mouseXCache +
                                    mPos.y - mouseYCache +
                                    1.;
                                glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.), glm::vec3(scale, scale, 1.));
                                scaleRotMatrix = scaleRotCache * transMatrix * scaleMatrix * glm::inverse(transMatrix);
                                fragInput.xform = 
                                    glm::inverse(pivotMatrix) *
                                    scaleRotMatrix * 
                                    transMatrix * 
                                    pivotMatrix * 
                                    toCanvasSpace;
                                auto cmd = renderPool.request(swapOnlyCmd, true);
                                pushCmd(std::move(cmd));
                                break;
                            }
                        case InputState::Translate:
                            {
                                mPos = toCanvasSpace * mPos - transMouseCache;
                                mPos *= -1;
                                transMatrix = 
                                    glm::translate(glm::mat4(1.), glm::vec3(mPos.x, mPos.y, 0.)) * 
                                    transCache;
                                fragInput.xform = 
                                    glm::inverse(pivotMatrix) * 
                                    scaleRotMatrix * 
                                    transMatrix * 
                                    pivotMatrix * 
                                    toCanvasSpace;
                                auto cmd = renderPool.request(swapOnlyCmd, true);
                                pushCmd(std::move(cmd));
                                break;
                            }
                        case InputState::BrushResize:
                            {
                                float brushSize = 
                                    mPos.x - mouseXCache +
                                    mPos.y - mouseYCache +
                                    brushSizeCache;
                                fragInput.brushSize = brushSize;
                                auto cmd = renderPool.request(brushSizeCmd, true);
                                pushCmd(std::move(cmd));
                                std::cout << "resizing " << brushSize << std::endl;
                                break;
                            }
                    }
                    break;
                }
            case WindowEventType::MousePress:
                {
                    auto input = static_cast<MousePressEvent*>(winevent);
                    mPos = glm::vec4{
                        input->getX()/ float(width), 
                        input->getY()/ float(width), 
                        0., 1.};
                    switch (input->getMouseButton())
                    {
                        case MouseButton::Left:
                            {
                                state = InputState::Paint;
                                mPos = fragInput.xform * mPos;
                                fragInput.mouseX = mPos.x;
                                fragInput.mouseY = mPos.y;
                                auto cmd = renderPool.request(paintCommand, true);
                                pushCmd(std::move(cmd));
                                break;
                            }
                        case MouseButton::Right:
                            {
                                mouseXCache = mPos.x;
                                mouseYCache = mPos.y;
                                state = InputState::Scale;
                                break;
                            }
                        case MouseButton::Middle:
                            {
                                transMouseCache = toCanvasSpace * mPos;
                                state = InputState::Translate;
                            }
                    }
                    break;
                }
            case WindowEventType::Keypress:
                {
                    auto input = static_cast<KeyPressEvent*>(winevent);
                    switch (static_cast<PressAction>(input->getKey()))
                    {
                        case PressAction::Rotate:
                            {
                                std::cout << "Entering rotation" << std::endl;
                                state = InputState::Rotate;
                                mouseXCache = angleScale * input->getX() / float(width);
                                mouseYCache = angleScale * input->getY() / float(height);
                                break;
                            }
                        case PressAction::Color:
                            {
//                                std::cout << "Loading color from pixel under cursor" << std::endl;
//                                vk::Rect2D region;
//                                region.offset.x = input->getX();
//                                region.offset.y = input->getY();
//                                region.extent.setWidth(1);
//                                region.extent.setHeight(1);
//                                auto block = renderer.copySwapToHost(region);
//                                uint8_t* ptr = static_cast<uint8_t*>(block->pHostMemory);
//                                std::cout << "B: " << static_cast<uint16_t>(*ptr) << std::endl;
//                                fragInput.b = float(*ptr++) / 255.;
//                                std::cout << "G: " << static_cast<uint16_t>(*ptr) << std::endl;
//                                fragInput.g = float(*ptr++) / 255.;
//                                std::cout << "R: " << static_cast<uint16_t>(*ptr) << std::endl;
//                                fragInput.r = float(*ptr++) / 255.;
//                                renderer.popBufferBlock();
//                                renderer.render(swapOnlyCmd, true);
                                break;
                            }
                        case PressAction::Alpha:
                            {
                                std::cout << "Enter an alpha value." << std::endl;
                                std::cin >> fragInput.a;
                                break;
                            }
                        case PressAction::BrushSize:
                            {
                                std::cout << "Choose brush size" << std::endl;
                                std::cin >> fragInput.brushSize;
                                break;
                            }
                        case PressAction::BrushInteractive:
                            {
                                mouseXCache = input->getX() / float(width);
                                mouseYCache = input->getY() / float(height);
                                state = InputState::BrushResize;
                                auto cmd = renderPool.request(brushSizeCmd, true);
                                pushCmd(std::move(cmd));
                                break;
                            }
                        case PressAction::Exit:
                            {
                                std::cout << "Closing application" << std::endl;
                            }
                        case PressAction::Save:
                            {
//                                std::cout << "Copying to buffer" << std::endl;
//                                vk::Rect2D region;
//                                region.offset.x = 0;
//                                region.offset.y = 0;
//                                region.extent.setWidth(C_WIDTH);
//                                region.extent.setHeight(C_HEIGHT);
//                                std::cout << "size: " << C_WIDTH * C_HEIGHT * 4 << std::endl;
//                                auto block = renderer.copyAttachmentToHost("paint0", region);
//                                const unsigned char* blockPtr = static_cast<const unsigned char*>(block->pHostMemory);
//                                std::vector<unsigned char> pngBuffer;
//                                lodepng::encode(
//                                        pngBuffer, 
//                                        blockPtr, 
//                                        C_WIDTH,
//                                        C_HEIGHT);
//                                std::string name;
//                                std::cout << "Please name your image." << std::endl;
//                                std::cin >> name;
//                                std::string path = "output/images/" + name + ".png";
//                                lodepng::save_file(pngBuffer, path);
//                                renderer.render(swapOnlyCmd, true);
//                                renderer.popBufferBlock();
                                break;
                            }
                        case PressAction::Palette:
                            {
                                auto cmd = renderPool.request(brushSizeCmd, true);
                                pushCmd(std::move(cmd));
                                break;
                            }
                    }
                    break;
                }
            case WindowEventType::Keyrelease:
                {
                    auto input = static_cast<KeyReleaseEvent*>(winevent);
                    switch (static_cast<ReleaseAction>(input->getKey()))
                    {
                        case ReleaseAction::Rotate:
                            {
                                std::cout << "Leaving rotation" << std::endl;
                                rotateDown = false;
                                scaleRotCache = scaleRotMatrix;
                                state = InputState::Idle;
                                break;
                            }
                        case ReleaseAction::BrushSize:
                            {
                                state = InputState::Idle;
                                brushSizeCache = fragInput.brushSize;
                                break;
                            }
                    }
                    break;
                }
            case WindowEventType::MouseRelease:
                {
                    transCache = transMatrix;
                    scaleRotCache = scaleRotMatrix;
                    state = InputState::Idle;
                    break;
                }
            case WindowEventType::LeaveWindow:
                {
                    rotateDown = false;
                    state = InputState::Idle;
                    break;
                }
            case WindowEventType::EnterWindow:
                {
                    break;
                }
        }
    }
}

void Paint::handleEvent(Event* event)
{
    if (event->getCategory() == EventCategory::Abort && mode != Mode::initial)
    {
        mode = Mode::initial;
        vocab = opMap.getStrings();
        event->setHandled();
    }
    else
        std::invoke(functions[static_cast<uint8_t>(mode)], this, event);
}

void Paint::onEnterExt(Application* app)
{
    app->renderer.bindUboData(static_cast<void*>(&fragInput), sizeof(FragmentInput), 0);
}

}

namespace Command
{

void AddAttachment::execute(Application* app)
{
        auto& attachment = app->renderer.createAttachment(attachmentName, dimensions, usage);
        if (usage & vk::ImageUsageFlagBits::eSampled)
            app->sampledImages.push_back(&attachment.getImage(0));
}

void LoadFragShader::execute(Application* app)
{
    app->renderer.loadFragShader(SHADER_DIR + shaderName, shaderName);
}

void LoadVertShader::execute(Application* app)
{
    app->renderer.loadVertShader(SHADER_DIR + shaderName, shaderName);
}

void SetSpecFloat::execute(Application* app)
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

void SetSpecInt::execute(Application* app)
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

void OpenWindow::execute(Application* app)
{
    app->window.open();
}

void PrepareRenderFrames::execute(Application* app)
{
    app->renderer.prepareRenderFrames(*window);
    std::cout << "Render frames prepared" << ENDL;
}

void CreateDescriptorSetLayout::execute(Application* app)
{
    app->renderer.createDescriptorSetLayout(name, bindings);
}

void CreateSwapchainRenderpass::execute(Application* app)
{
    auto& rpass = app->renderer.createRenderPass(rpassName);
    app->renderer.prepareAsSwapchainPass(rpass);
}

void CreateOffscreenRenderpass::execute(Application* app)
{
    auto& rpass = app->renderer.createRenderPass(rpassName);
    app->renderer.prepareAsOffscreenPass(rpass, loadOp);
}

void CreateGraphicsPipeline::execute(Application* app)
{
    auto& pipeline = app->renderer.createGraphicsPipeline(
            name, pipelineLayout,
            vertshader, fragshader,
            renderpass, renderArea, is3d);
    pipeline.create();
}

void CreateRenderpassInstance::execute(Application* app)
{
    app->renderer.addRenderPassInstance(attachment, renderpass, pipeline);
}

void RecordRenderCommand::execute(Application* app)
{
    app->renderer.recordRenderCommands(cmdBufferId, renderpassInstances);
}

void Render::execute(Application* app)
{
    app->renderer.render(renderCommandId, updateUBO);
}

void RefreshState::execute(Application* app)
{
    state->refresh(app);
}

void UpdateVocab::execute(Application* app)
{
    app->ev.updateVocab();
}

void CreatePipelineLayout::execute(Application* app)
{
    app->renderer.createPipelineLayout(name, descriptorSetLayoutNames);
}

void CreateFrameDescriptorSets::execute(Application* app)
{
    app->renderer.createFrameDescriptorSets(layoutnames);
}

void InitFrameUbos::execute(Application* app)
{
    app->renderer.initFrameUBOs(sizeof(app->fragInput), binding);
}

void UpdateFrameSamplers::execute(Application* app)
{
    app->renderer.updateFrameSamplers(app->sampledImages, binding);
}

}

Application::Application(uint16_t w, uint16_t h, const std::string logfile, int eventPops) :
    window{w, h},
    ev{window},
    renderer{context},
    offscreenDim{{w, h}},
    swapDim{{w, h}},
    readlog{logfile},
    dirState{window, stateStack, stateEdits, cmdStack}
{
    stateStack.push(&dirState);
    dirState.onEnter(this);
    ev.updateVocab();
    if (readlog != "eventlog")
    {
        std::cout << "reading events from " << readlog << std::endl;
        recordevents = true;
        readevents = true;
    }
    stateStack.top()->onEnter(this);
    
    if (readevents) readEvents(is, eventPops);
    if (recordevents) std::remove(writelog.data());
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

void Application::setVocabulary(State::Vocab vocab)
{
    ev.setVocabulary(*vocab.getValues());
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
    std::cout << "Created Pipeline Layout: " << layoutName << ENDL;
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

void Application::recordEvent(Event* event, std::ofstream& os)
{
    if (event->getCategory() == EventCategory::CommandLine)
    {

        os.open(writelog, std::ios::out | std::ios::binary | std::ios::app);
        auto ce = static_cast<CommandLineEvent*>(event);
        ce->serialize(os);
        os.close();
    }
    if (event->getCategory() == EventCategory::Abort)
    {

        os.open(writelog, std::ios::out | std::ios::binary | std::ios::app);
        auto ce = static_cast<Abort*>(event);
        ce->serialize(os);
        os.close();
    }
}

void Application::readEvents(std::ifstream& is, int eventPops)
{
    is.open(readlog, std::ios::binary);
    bool reading{true};
    while (is.peek() != EOF)
    {
        EventCategory ec = Event::unserializeCategory(is);
        if (ec == EventCategory::CommandLine)
        {
            auto cmdevnt = std::make_unique<CommandLineEvent>("foo");
            cmdevnt->unserialize(is);
            ev.eventQueue.push(std::move(cmdevnt));
            std::cout << "pushed commandline event" << ENDL;
        }
        if (ec == EventCategory::Abort)
        {
            auto abrt = std::make_unique<Abort>();
            ev.eventQueue.push(std::move(abrt));
            std::cout << "pushed abort event" << ENDL;
        }
    }
    for (int i = 0; i < eventPops; i++) 
    {
        ev.eventQueue.pop();
        std::cout << "popped from queue" << std::endl;
    }
    is.close();
}

void Application::run()
{
    ev.pollEvents();

    while (1)
    {
        for (const auto& event : ev.eventQueue) 
        {
            if (event)
            {
                if (recordevents) recordEvent(event.get(), os);
                for (auto state : stateStack) 
                    if (!event->isHandled())
                        state->handleEvent(event.get());
            }
            for (auto state : stateEdits) 
            {
                if (state)
                    pushState(state);
                else
                    popState();
            }
            stateEdits.clear();
        }
        for (auto& cmd : cmdStack.items)
        {
            if (cmd)
            {
                cmd->execute(this);
                std::cout << "Command run: " << cmd->getName() << ENDL;
            }
            else
                std::cout << "Recieved null cmd" << std::endl;
        }
        ev.eventQueue.items.clear();
        cmdStack.items.clear();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

