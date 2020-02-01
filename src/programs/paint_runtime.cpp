#define GLM_ENABLE_EXPERIMENTAL
#include "../core/window.hpp"
#include "../core/renderer.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "../core/util.hpp"
#include <unistd.h>
#include "../core/event.hpp"
#include <glm/gtx/matrix_transform_2d.hpp>
#include "../thirdparty/lodepng.h"
#include <map>
#include <sstream>
#include <readline/readline.h>
#include <readline/history.h>
#include <fstream>
#include <algorithm>
#include <cctype>

#define FETCHINPUT(); std::string input = fetchInput(); if (input == "") return;

constexpr uint32_t N_FRAMES = 10000000;
constexpr uint32_t WIDTH = 1400;
constexpr uint32_t HEIGHT = 1000;
constexpr uint32_t C_WIDTH = 4096;
constexpr uint32_t C_HEIGHT = 4096;

constexpr char SHADER_DIR[] = "build/shaders/";
Timer myTimer;

struct RenderPassInstanceInfo
{
    uint32_t index;
    std::string attachmentName;
    std::string renderPassName;
    std::string pipelineName;
};

struct CommandBufferInfo
{
    uint32_t bufferId;
    std::vector<uint32_t> renderPassInstances;
};

struct FragmentInput
{
    float time;
	float mouseX{0};
	float mouseY{0};	
    float r{1.};
    float g{1.};
    float b{1.};
    float a{1.};
    float brushSize{1.};
    glm::mat4 xform{1.};
};

struct PaintData
{
    float angleScale{3};
    glm::mat4 transCache{1.};
    glm::mat4 scaleRotCache{1.};
    glm::mat4 scaleCache{1.};
    bool rotateDown{false};
    float scale{1};
    float mouseXCache{0};
    float mouseYCache{0};
    float angle{0};
    float angleInitCache{0};
    float brushSizeCache{1.};
    glm::vec4 mPos{0};
    glm::vec4 transMouseCache{0};
};

enum class PressAction : uint8_t 
{
    Rotate = static_cast<uint8_t>(Key::Alt),
    Color = static_cast<uint8_t>(Key::C),
    BrushSize = static_cast<uint8_t>(Key::B),
    BrushInteractive = static_cast<uint8_t>(Key::Shift_L),
    Exit = static_cast<uint8_t>(Key::Esc),
    Save = static_cast<uint8_t>(Key::S),
    Alpha = static_cast<uint8_t>(Key::A),
    Palette = static_cast<uint8_t>(Key::P)
};

enum class PaintState : uint8_t
{
    Paint,
    Rotate,
    Scale, 
    Translate,
    Idle,
    BrushResize
};

enum class InputState : uint8_t
{
    Command,
    Paint,
};

enum class ReleaseAction : uint8_t 
{
    Rotate = static_cast<uint8_t>(PressAction::Rotate),
    BrushSize = static_cast<uint8_t>(PressAction::BrushInteractive)
};


constexpr const char* usageStatement =
R"(
===PROGRAM EXECUTION GUIDE===
Steps must be performed in the correct order or bad things will happen.
Use the command listcommands to view possible commands.

0. Open window.
0. Initialize Renderer.
0. Set offscreen dimensions
0. Load shaders
   Create attachments
0. Set shader specialization infos
   Create Pipelines

)";


bool parsedQuit(std::string input)
{
    std::stringstream ss{input};
    std::string cur;
    while (!ss.eof())
    {
        ss >> cur;
        if (cur.compare("q") == 0)
        {
            std::cout << "Parsed quit" << std::endl;
            return true;
        }
    }
    return false;
}

class Runtime;

typedef void (Runtime::*rtFunc)();

class Runtime
{
public:
    Runtime(uint16_t w, uint16_t h);
    virtual ~Runtime();

    void setSpecData();
    void addAttachment();
    void setOffscreenDim();
    void loadFragShaders();
    void createPipeline();
    void createRenderPassInstance();
    void recordRenderCommand();
    void listCommands();
    void listPipelineLayouts();
    void listAttachments();
    void listFragShaders();
    void listVertShaders();
    void listInstructions();
    void listShadersOnDisk();
    void listRenderPasses();
    void report();
    void openWindow();
    void initializeRenderer();
    void prepareRenderFrames();
    void printDimensions();
    void renderFrame();
    void updateSamplers();
    void listSampledImages();

    void run(const char* loadFile);
    void paint();

protected:
    void createPipelineLayout();
    void loadDefaultShaders();
    std::string fetchInput();
    void initUBO();
    rtFunc findCommand(std::string) const;

private:
    Context context;
    XWindow window;
    Renderer renderer;
    EventHandler eventHandler;
    std::ofstream outfile;
    std::ifstream infile;
    glm::uvec2 swapDim{256, 256};
    glm::uvec2 offscreenDim{256, 256};
    std::vector<const mm::Image*> sampledImages;
    const std::string shaderDir{SHADER_DIR};
    uint16_t renderPassInstanceCount;
    std::vector<RenderPassInstanceInfo> rpassInfos;
    std::vector<CommandBufferInfo> cmdBufferInfos;

    FragmentInput fragInput;
    PaintData paintData;

    InputState inputState{InputState::Command};
};

enum class CommandId : uint8_t
{
    ListCommands,
    ListInstructions,
    ListShadersOnDisk,
    ListAttachments,
    ListFragShaders,
    ListVertShaders,
    ListRenderPasses,
    ListPipelineLayouts,
    Report,
    SetSpecData,
    AddAttachment,
    SetOffscreenDim,
    OpenWindow,
    InitializeRenderer,
    PrintDimensions,
    LoadFragShader,
    CreatePipeline,
    CreateRenderPassInstance,
    RecordRenderCommand,
    RenderFrame,
    UpdateSamplers,
    ListSampledImages,
    Paint,
};

struct Command
{
    CommandId id;
    const char* name;
    const char* label;
    rtFunc fn;
};

std::vector<Command> commands = 
{
    {CommandId::AddAttachment,              "addAttachment",             "Add Attachment",                         &Runtime::addAttachment},
    {CommandId::SetSpecData,                "setSpecData",               "Set Specialization Info",                &Runtime::setSpecData},
    {CommandId::ListAttachments,            "listAttachments",           "List Attachments",                       &Runtime::listAttachments},
    {CommandId::ListFragShaders,            "listFragShaders",           "List Loaded Fragment Shaders",           &Runtime::listFragShaders},
    {CommandId::ListCommands,               "listCommands",              "List Commands",                          &Runtime::listCommands},
    {CommandId::ListInstructions,           "listInstructions",          "List Instructions",                      &Runtime::listInstructions},
    {CommandId::OpenWindow,                 "openWindow",                "Open Window",                            &Runtime::openWindow},
    {CommandId::InitializeRenderer,         "initRenderer",              "Initialize Renderer.",                   &Runtime::initializeRenderer}, 
    {CommandId::SetOffscreenDim,            "setOffscreenDim",           "Set Offscreen Dimensions.",              &Runtime::setOffscreenDim}, 
    {CommandId::PrintDimensions,            "printDimensions",           "Print Offscreen and Swap dimensions.",   &Runtime::printDimensions},
    {CommandId::ListShadersOnDisk,          "listShadersOnDisk",         "List shaders you can load from disk.",   &Runtime::listShadersOnDisk}, 
    {CommandId::LoadFragShader,             "loadFragShaders",           "Load Fragment Shader from disk.",        &Runtime::loadFragShaders},             
    {CommandId::ListRenderPasses,           "listRenderPasses",          "List Render Passes available.",          &Runtime::listRenderPasses}, 
    {CommandId::CreatePipeline,             "createPipeline",            "Create a graphics pipeline object.",     &Runtime::createPipeline},                
    {CommandId::CreateRenderPassInstance,   "createRenderPassInstance",  "Create a render pass instance.",         &Runtime::createRenderPassInstance},
    {CommandId::ListPipelineLayouts,        "listPipelineLayouts",       "List pipeline layouts.",                 &Runtime::listPipelineLayouts},
    {CommandId::ListVertShaders,            "listVertShaders",           "List loaded vertex shaders.",            &Runtime::listVertShaders},
    {CommandId::RecordRenderCommand,        "recordRenderCommand",       "Record render command.",                 &Runtime::recordRenderCommand},
    {CommandId::RenderFrame,                "renderFrame",               "Render single frame.",                   &Runtime::renderFrame},
    {CommandId::Report,                     "report",                    "Report all.",                            &Runtime::report},
    {CommandId::UpdateSamplers,             "updateSamplers",            "Update descriptor set samplers.",        &Runtime::updateSamplers},
    {CommandId::ListSampledImages,          "listSampledImages",         "List Sampled Images",                    &Runtime::listSampledImages},
    {CommandId::Paint,                      "paint",                     "Enter paint state.",                     &Runtime::paint},
};


char* completion_generator(const char* text, int state)
{
    static std::vector<std::string> matches;
    static size_t match_index = 0;

    if (state == 0)
    {
        matches.clear();
        match_index = 0;
    
        std::string textstr(text);
        for (auto cmd : commands)
        {
            std::string word(cmd.name);
            if (word.size() >= textstr.size() && word.compare(0, textstr.size(), textstr) == 0)
            {
                matches.push_back(word);
            }
        }
    }
    
    if (match_index >= matches.size())
    {
        return nullptr;
    }
    else
    {
        return strdup(matches[match_index++].c_str());
    }
}

char** completer(const char* text, int start, int end)
{
    rl_attempted_completion_over = 1;

    return rl_completion_matches(text, completion_generator);
}

Runtime::Runtime(uint16_t w, uint16_t h) :
    context{},
    window{w, h},
    eventHandler{window},
    renderer{context},
    swapDim{w, h},
    offscreenDim{w, h}
{
    std::cout << usageStatement << std::endl;
    listCommands();
}

Runtime::~Runtime()
{
}

std::string Runtime::fetchInput()
{
    std::string input;
    if (std::getline(infile, input))
    {
        return input;
    }
    rl_attempted_completion_function = completer;

    char* buf = readline(">> ");

    if (buf && *buf) //dont add empty lines to history
        add_history(buf);

    input = std::string(buf);
    if (parsedQuit(input))
        return "";

    if (input == "save")
    {
        std::cout << "Enter save file name." << std::endl;
        std::string savefilename;
        std::cin >> savefilename;
        std::ofstream savefile(savefilename);
        std::ifstream curOut("log");
        savefile << curOut.rdbuf();
        return "";
    }
    outfile << input;
    outfile << std::endl;

    free(buf);
    std::cout << "Input check " << input << std::endl;

    return input;
}

void Runtime::listCommands()
{
    std::cout << "Available commands." << std::endl;
    for (const auto cmd : commands) 
    {
       std::cout << cmd.label << std::endl;
    }
    std::cout << std::endl;
}

void Runtime::listAttachments()
{
    renderer.listAttachments();
}

void Runtime::listFragShaders()
{
    renderer.listFragShaders();
}

void Runtime::listVertShaders()
{
    renderer.listVertShaders();
}

void Runtime::listPipelineLayouts()
{
    renderer.listPipelineLayouts();
}

void Runtime::loadDefaultShaders()
{
    //vert shaders
    auto& a = renderer.loadVertShader(shaderDir + "fullscreen_tri.spv", "vert1");
    auto& b = renderer.loadFragShader(shaderDir + "passthrough.spv", "swap");
    b.setWindowResolution(swapDim.x, swapDim.y);
}

void Runtime::loadFragShaders()
{
    std::cout << "Enter shader names" << std::endl;
    FETCHINPUT();
    std::vector<std::string> shaderNames;
    std::string holder;
    std::stringstream instream{input};
    while (instream >> holder)
        shaderNames.push_back(holder);
    for (auto& shaderName : shaderNames) 
    {
        auto& shader = renderer.loadFragShader(shaderDir + shaderName + ".spv", shaderName);
        shader.setWindowResolution(offscreenDim.x, offscreenDim.y);
    }
}

void Runtime::setSpecData()
{
    std::cout << "Enter he first spec float, the second spec float, " << 
        "the first spec int, the second spec int. Then enter the shaders to apply it to." << std::endl;
    FETCHINPUT();
    SpecData data;
    std::stringstream instream{input};
    bool success = static_cast<bool>(instream >> data.windowWidth >> data.windowHeight >> data.integer0 >> data.integer1);
    if (!success) {std::cout << "Missing entries." << std::endl; return;}
    std::vector<std::string> names;
    std::string holder;
    while (instream >> holder)
        names.push_back(holder);
    for (auto& name : names)
        renderer.fragShaderAt(name).specData = data;
}

void Runtime::addAttachment()
{
    std::cout << "Enter a name, 1 if it is to be sampled, 1 if it is to be a transfer src." << std::endl;
    FETCHINPUT();
    std::string name;
    bool isSampled, isTransferSrc;
    std::stringstream instream{input};
    bool success = static_cast<bool>(instream >> name >> isSampled >> isTransferSrc);
    if (!success) {std::cout << "Missing entries." << std::endl; return;}
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
    if (isSampled)
        usage = usage | vk::ImageUsageFlagBits::eSampled;
    if (isTransferSrc)
        usage = usage | vk::ImageUsageFlagBits::eTransferSrc;
    auto& attachment = renderer.createAttachment(
            name, {offscreenDim.x, offscreenDim.y}, usage);
    if (isSampled)
        sampledImages.push_back(&attachment.getImage(0));
}

void Runtime::setOffscreenDim()
{
    std::cout << "Enter a width and a height for offscreen attachments." << std::endl;
    FETCHINPUT();
    float w, h;
    std::stringstream instream{input};
    bool success = static_cast<bool>(instream >> w >> h);
    if (!success) {std::cout << "Missing entries." << std::endl; return;}
    offscreenDim = glm::uvec2{w, h};
}

void Runtime::createPipelineLayout()
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

void Runtime::createPipeline()
{
    std::cout << 
        "Enter pipeline name, "
        "pipeline layout name, "
        "vert shader name, "
        "fragment shader name, "
        "renderpass name, "
        "1 if offscreen size "
        << std::endl;
    FETCHINPUT();
    std::string name, layoutname, vshader, fshader, rpass;
    bool isOffscreen;
    std::stringstream instream{input};
    bool success = static_cast<bool>(instream >> name >> layoutname >> vshader >> fshader >> rpass >> isOffscreen);
    if (!success) {std::cout << "Missing entries." << std::endl; return;}

    vk::Extent2D area;
    if (isOffscreen)
        area = vk::Extent2D{offscreenDim.x, offscreenDim.y};
    else
        area = vk::Extent2D{swapDim.x, swapDim.y};
    vk::Rect2D region = {{0,0}, area};
    auto& pipeline = renderer.createGraphicsPipeline(
            name, layoutname, vshader, fshader, rpass, region, false);
    pipeline.create();
}

void Runtime::initUBO()
{
    renderer.initFrameUBOs(sizeof(FragmentInput), 0); //should be a vector
    renderer.bindUboData(static_cast<void*>(&fragInput), sizeof(FragmentInput), 0);
}

void Runtime::updateSamplers()
{
    renderer.updateFrameSamplers(sampledImages, 1); //image vector, binding number
}

void Runtime::listSampledImages()
{
    for (auto& i : sampledImages) 
    {
        std::cout << "Image at: " << i << std::endl;   
    }
}

void Runtime::createRenderPassInstance()
{
    std::cout << "Enter an attachment name (or swap), a renderpass name, a pipeline name." << std::endl;
    FETCHINPUT();
    std::string attachment, renderpass, pipeline;
    std::stringstream instream{input};
    bool success = static_cast<bool>(instream >> attachment >> renderpass >> pipeline);
    if (!success) {std::cout << "Missing entries." << std::endl; return;}
    renderer.addRenderPassInstance(attachment, renderpass, pipeline);

    RenderPassInstanceInfo info{renderPassInstanceCount, attachment, renderpass, pipeline};
    rpassInfos.push_back(info);
}

void Runtime::recordRenderCommand()
{
    std::cout << "Enter an index for the command buffer and an array for render pass instances." << std::endl;
    FETCHINPUT();
    uint8_t commandId;
    std::vector<u_int32_t> renderPassInstances; 
    std::stringstream instream{input};
    instream >> commandId;
    while (!instream.eof())
    {
        uint32_t id;
        instream >> id;
        renderPassInstances.push_back(id);
    }
    renderer.recordRenderCommands(commandId, renderPassInstances);

    CommandBufferInfo info = {commandId, renderPassInstances};
    cmdBufferInfos.push_back(info);
}

void Runtime::listInstructions()
{
    std::cout << usageStatement << std::endl;
}

void Runtime::listShadersOnDisk()
{
    auto command = std::string("ls " + shaderDir);
    std::system(command.c_str());
}

void Runtime::listRenderPasses()
{
    renderer.listRenderPasses();
}

void Runtime::openWindow()
{
    window.open();
}

void Runtime::prepareRenderFrames()
{
    renderer.prepareRenderFrames(window);
    createPipelineLayout();
    std::cout << "Render frames prepared." << std::endl;
    std::cout << "Pipeline layout created as well." << std::endl;
}

void Runtime::printDimensions()
{
    std::cout << "Current offscreen dimension." << std::endl;
    std::cout << offscreenDim.x << " " << offscreenDim.y << std::endl;
    std::cout << "Current swapchain dimension." << std::endl;
    std::cout << swapDim.x << " " << swapDim.y << std::endl;
}

void Runtime::report()
{
    listVertShaders();
    listFragShaders();
    listAttachments();
    listPipelineLayouts();
    listRenderPasses();
}

void Runtime::initializeRenderer()
{
    prepareRenderFrames();
    loadDefaultShaders();
    auto& offScreenLoadPass = renderer.createRenderPass("offscreen_load");
    renderer.prepareAsOffscreenPass(offScreenLoadPass, vk::AttachmentLoadOp::eLoad); //can make this a static function of RenderPass
    auto& swapchainPass = renderer.createRenderPass("swapchain");
    renderer.prepareAsSwapchainPass(swapchainPass); //can make this a static function of RenderPass
    auto& offScreenClearPass = renderer.createRenderPass("offscreen_clear");
    renderer.prepareAsOffscreenPass(offScreenClearPass, vk::AttachmentLoadOp::eClear);
    initUBO();
}

void Runtime::renderFrame()
{
    std::cout << "Enter command buffer id, 1 to update Ubo." << std::endl;
    FETCHINPUT();
    uint32_t id;
    bool update;
    std::stringstream instream{input};
    instream >> id >> update;
    renderer.render(id, update);
}

rtFunc Runtime::findCommand(const std::string input) const
{
    for (auto& item : commands) 
    {
        if (input.compare(item.name) == 0)
            return item.fn;
    }
    std::cout << "Invalid command" << std::endl;
    return nullptr;
}

void Runtime::run(const char* loadFile)
{
    outfile.open("log", std::ofstream::out | std::ofstream::trunc);
    if (loadFile)
    {
        std::cout << "Loading cached state" << std::endl;
        infile.open(loadFile);
    }
    while (1)
    {
        FETCHINPUT(); //creates an input variable for us
        input.erase(std::remove(input.begin(), input.end(), ' '), input.end());
        rtFunc cmd = findCommand(input);
        if (cmd == nullptr)
            continue;
        else
            (*this.*cmd)();
    }
}

void Runtime::paint()
{
    const uint8_t paintCommand = 0;
    const uint8_t swapOnlyCmd = 1;
    const uint8_t paletteCmd = 2;
    const uint8_t brushSizeCmd = 3;

    float cMapX = float(WIDTH)/float(C_WIDTH);
    float cMapY = float(HEIGHT)/float(C_HEIGHT);

    fragInput.xform = glm::scale(glm::mat4(1.), glm::vec3(cMapX, cMapY, 1.0));

    glm::mat4 toCanvasSpace = glm::scale(glm::mat4(1.), glm::vec3(cMapX, cMapY, 1.0)); 
    glm::mat4 transMatrix{1.};
    glm::mat4 pivotMatrix = glm::translate(glm::mat4(1.), glm::vec3{-0.5 * cMapX, -0.5 * cMapY, 0.0});
    glm::mat4 scaleRotMatrix{1.};
    fragInput.xform = toCanvasSpace;


    PaintState state{PaintState::Idle};

    for (int i = 0; i < N_FRAMES; i++) 
    {
        auto& input = eventHandler.fetchUserInput(true);
        switch (input.eventType)
        {
            case EventType::Motion:
                {
                    glm::vec4 mPos = glm::vec4{
                        input.mouseX / float(WIDTH), input.mouseY / float(HEIGHT), 1., 1.};
                    switch (state)
                    {
                        case PaintState::Paint:
                            {
                                mPos = fragInput.xform * mPos;
                                fragInput.mouseX = mPos.x;
                                fragInput.mouseY = mPos.y;
                                myTimer.start();
                                renderer.render(paintCommand, true);
                                myTimer.end("Render");
                                break;
                            }
                        case PaintState::Idle:
                            {
                                mPos = fragInput.xform * mPos;
                                fragInput.mouseX = mPos.x;
                                fragInput.mouseY = mPos.y;
                                renderer.render(brushSizeCmd, true);
                                break;
                            }
                        case PaintState::Rotate:
                            {
                                paintData.angle = 
                                    paintData.angleScale * input.mouseX / float(WIDTH) - paintData.mouseXCache + 
                                    paintData.angleScale * input.mouseY / float(HEIGHT) - paintData.mouseYCache;
                                scaleRotMatrix = paintData.scaleRotCache * transMatrix * glm::rotate(glm::mat4(1.), paintData.angle, {0, 0, 1.}) * glm::inverse(transMatrix);
                                fragInput.xform = 
                                    glm::inverse(pivotMatrix) * 
                                    scaleRotMatrix * 
                                    transMatrix * 
                                    pivotMatrix * 
                                    toCanvasSpace;
                                renderer.render(swapOnlyCmd, true);
                                break;
                            }
                        case PaintState::Scale:
                            {
                                paintData.scale = 
                                    mPos.x - paintData.mouseXCache +
                                    mPos.y - paintData.mouseYCache +
                                    1.;
                                glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.), glm::vec3(paintData.scale, paintData.scale, 1.));
                                scaleRotMatrix = paintData.scaleRotCache * transMatrix * scaleMatrix * glm::inverse(transMatrix);
                                fragInput.xform = 
                                    glm::inverse(pivotMatrix) *
                                    scaleRotMatrix * 
                                    transMatrix * 
                                    pivotMatrix * 
                                    toCanvasSpace;
                                renderer.render(swapOnlyCmd, true);
                                break;
                            }
                        case PaintState::Translate:
                            {
                                mPos = toCanvasSpace * mPos - paintData.transMouseCache;
                                mPos *= -1;
                                transMatrix = 
                                    glm::translate(glm::mat4(1.), glm::vec3(mPos.x, mPos.y, 0.)) * 
                                    paintData.transCache;
                                fragInput.xform = 
                                    glm::inverse(pivotMatrix) * 
                                    scaleRotMatrix * 
                                    transMatrix * 
                                    pivotMatrix * 
                                    toCanvasSpace;
                                renderer.render(swapOnlyCmd, true);
                                break;
                            }
                        case PaintState::BrushResize:
                            {
                                float brushSize = 
                                    mPos.x - paintData.mouseXCache +
                                    mPos.y - paintData.mouseYCache +
                                    paintData.brushSizeCache;
                                fragInput.brushSize = brushSize;
                                renderer.render(brushSizeCmd, true);
                                std::cout << "resizing " << brushSize << std::endl;
                                break;
                            }
                    }
                    break;
                }
            case EventType::MousePress:
                {
                    paintData.mPos = glm::vec4{
                        input.mouseX / float(WIDTH), 
                        input.mouseY / float(HEIGHT), 
                        0., 1.};
                    switch (input.mouseButton)
                    {
                        case MouseButton::Left:
                            {
                                state = PaintState::Paint;
                                paintData.mPos = fragInput.xform * paintData.mPos;
                                fragInput.mouseX = paintData.mPos.x;
                                fragInput.mouseY = paintData.mPos.y;
                                renderer.render(paintCommand, true);
                                break;
                            }
                        case MouseButton::Right:
                            {
                                paintData.mouseXCache = paintData.mPos.x;
                                paintData.mouseYCache = paintData.mPos.y;
                                state = PaintState::Scale;
                                break;
                            }
                        case MouseButton::Middle:
                            {
                                paintData.transMouseCache = toCanvasSpace * paintData.mPos;
                                state = PaintState::Translate;
                            }
                    }
                    break;
                }
            case EventType::Keypress:
                {
                    switch (static_cast<PressAction>(input.key))
                    {
                        case PressAction::Rotate:
                            {
                                std::cout << "Entering rotation" << std::endl;
                                state = PaintState::Rotate;
                                paintData.mouseXCache = paintData.angleScale * input.mouseX / float(WIDTH);
                                paintData.mouseYCache = paintData.angleScale * input.mouseY / float(HEIGHT);
                                break;
                            }
                        case PressAction::Color:
                            {
                                std::cout << "Loading color from pixel under cursor" << std::endl;
                                vk::Rect2D region;
                                region.offset.x = input.mouseX;
                                region.offset.y = input.mouseY;
                                region.extent.setWidth(1);
                                region.extent.setHeight(1);
                                auto block = renderer.copySwapToHost(region);
                                uint8_t* ptr = static_cast<uint8_t*>(block->pHostMemory);
                                std::cout << "B: " << static_cast<uint16_t>(*ptr) << std::endl;
                                fragInput.b = float(*ptr++) / 255.;
                                std::cout << "G: " << static_cast<uint16_t>(*ptr) << std::endl;
                                fragInput.g = float(*ptr++) / 255.;
                                std::cout << "R: " << static_cast<uint16_t>(*ptr) << std::endl;
                                fragInput.r = float(*ptr++) / 255.;
                                renderer.popBufferBlock();
                                renderer.render(swapOnlyCmd, true);
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
                                paintData.mouseXCache = input.mouseX / float(WIDTH);
                                paintData.mouseYCache = input.mouseY / float(HEIGHT);
                                state = PaintState::BrushResize;
                                renderer.render(brushSizeCmd, true);
                                break;
                            }
                        case PressAction::Exit:
                            {
                                std::cout << "Closing application" << std::endl;
                                return;
                            }
                        case PressAction::Save:
                            {
                                std::cout << "Copying to buffer" << std::endl;
                                vk::Rect2D region;
                                region.offset.x = 0;
                                region.offset.y = 0;
                                region.extent.setWidth(C_WIDTH);
                                region.extent.setHeight(C_HEIGHT);
                                std::cout << "size: " << C_WIDTH * C_HEIGHT * 4 << std::endl;
                                auto block = renderer.copyAttachmentToHost("paint0", region);
                                const unsigned char* blockPtr = static_cast<const unsigned char*>(block->pHostMemory);
                                std::vector<unsigned char> pngBuffer;
                                lodepng::encode(
                                        pngBuffer, 
                                        blockPtr, 
                                        C_WIDTH,
                                        C_HEIGHT);
                                std::string name;
                                std::cout << "Please name your image." << std::endl;
                                std::cin >> name;
                                std::string path = "output/images/" + name + ".png";
                                lodepng::save_file(pngBuffer, path);
                                renderer.render(swapOnlyCmd, true);
                                renderer.popBufferBlock();
                                break;
                            }
                        case PressAction::Palette:
                            {
                                renderer.render(paletteCmd, true);
                                break;
                            }
                    }
                    break;
                }
            case EventType::Keyrelease:
                {
                    switch (static_cast<ReleaseAction>(input.key))
                    {
                        case ReleaseAction::Rotate:
                            {
                                std::cout << "Leaving rotation" << std::endl;
                                paintData.rotateDown = false;
                                paintData.scaleRotCache = scaleRotMatrix;
                                state = PaintState::Idle;
                                break;
                            }
                        case ReleaseAction::BrushSize:
                            {
                                state = PaintState::Idle;
                                paintData.brushSizeCache = fragInput.brushSize;
                                break;
                            }
                    }
                    break;
                }
            case EventType::MouseRelease:
                {
                    paintData.transCache = transMatrix;
                    paintData.scaleRotCache = scaleRotMatrix;
                    state = PaintState::Idle;
                    break;
                }
            case EventType::LeaveWindow:
                {
                    paintData.rotateDown = false;
                    state = PaintState::Idle;
                    break;
                }
            case EventType::EnterWindow:
                {
                    break;
                }
        }
    }
    return;
}

int main(int argc, char *argv[])
{
    Runtime app{800, 800};
    char* loadFile{nullptr};
    if (argc == 2)
        loadFile = argv[1];
    app.run(loadFile);
    return 0;
}

