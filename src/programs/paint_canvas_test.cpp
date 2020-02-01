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

constexpr uint32_t N_FRAMES = 10000000;
constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 800;
constexpr uint32_t C_WIDTH = 800;
constexpr uint32_t C_HEIGHT = 800;

constexpr char SHADER_DIR[] = "build/shaders/";
Timer myTimer;

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

enum class InputState : uint8_t
{
    Paint,
    Rotate,
    Scale, 
    Translate,
    Idle,
    BrushResize
};

enum class ReleaseAction : uint8_t 
{
    Rotate = static_cast<uint8_t>(PressAction::Rotate),
    BrushSize = static_cast<uint8_t>(PressAction::BrushInteractive)
};

int main(int argc, char *argv[])
{
    const std::string shaderDir{SHADER_DIR};

    Context context;
    XWindow window(WIDTH, HEIGHT);
    window.open();
    EventHandler eventHander(window);

    Renderer renderer(context);
    renderer.prepareRenderFrames(window);

    auto& paint0 = renderer.createAttachment("paint0", {C_WIDTH, C_HEIGHT}, 
            vk::ImageUsageFlagBits::eColorAttachment |
            vk::ImageUsageFlagBits::eSampled |
            vk::ImageUsageFlagBits::eTransferSrc);

    //setup descriptor set for painting pipeline
    std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
    bindings[0].setBinding(0);
    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    bindings[0].setDescriptorCount(1);
    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    bindings[1].setBinding(1);
    bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    bindings[1].setDescriptorCount(20);
    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    auto paintingLayout = renderer.createDescriptorSetLayout("paintLayout", bindings);

    renderer.createFrameDescriptorSets({paintingLayout}); //order matters for access
    
    auto paintPLayout = renderer.createPipelineLayout("paintpipelayout", {paintingLayout});

    auto sampledImages = std::vector<const mm::Image*>{
            &paint0.getImage(0)};

    for (auto& i : sampledImages) {
        std::cout << "Image: " << i << std::endl;
    }
    
    //vert shaders
    auto& quadShader = renderer.loadVertShader(shaderDir + "fullscreen_tri.spv", "vert1");

    //frag shaders
    auto& tarotShader = renderer.loadFragShader(shaderDir + "spot_xform.spv", "frag1");
    auto& swapShader = renderer.loadFragShader(shaderDir + "composite_xform.spv", "swap");
    auto& paletteShader = renderer.loadFragShader(shaderDir + "uv_00.spv", "uv00");
    auto& voidShader = renderer.loadFragShader(shaderDir + "void.spv", "void");

    tarotShader.setWindowResolution(C_WIDTH, C_HEIGHT);
    voidShader.setWindowResolution(C_WIDTH, C_HEIGHT);
    swapShader.setWindowResolution(WIDTH, HEIGHT); 
    paletteShader.setWindowResolution(WIDTH, HEIGHT);

    swapShader.specData.integer0 = 0; 
    swapShader.specData.integer1 = 0; 
    
    //we should actually not be doing this. the attachment size make sense for a spec constant
    //but the array indices would be better handled by push constants

    //create render passes
    auto& offScreenLoadPass = renderer.createRenderPass("offscreen");
    renderer.prepareAsOffscreenPass(offScreenLoadPass, vk::AttachmentLoadOp::eLoad); //can make this a static function of RenderPass
    auto& swapchainPass = renderer.createRenderPass("swapchain");
    renderer.prepareAsSwapchainPass(swapchainPass); //can make this a static function of RenderPass
    auto& offScreenClearPass = renderer.createRenderPass("offscreen_clear");
    renderer.prepareAsOffscreenPass(offScreenClearPass, vk::AttachmentLoadOp::eClear);
    
    //create pipelines
    vk::Rect2D swapRenderArea{{0,0}, {WIDTH, HEIGHT}};
    vk::Rect2D offscreenRenderArea{{0,0}, {C_WIDTH, C_HEIGHT}};
    auto& offscreenPipe = renderer.createGraphicsPipeline(
            "offscreen", paintPLayout, "vert1", "frag1", "offscreen", offscreenRenderArea, false);
    auto& swapchainPipe = renderer.createGraphicsPipeline(
            "swap", paintPLayout, "vert1", "frag1", "swapchain", swapRenderArea, false);

    offscreenPipe.create();
    swapchainPipe.create();
    
    //ubo stuff must update the sets before recording the command buffers

    renderer.initFrameUBOs(sizeof(FragmentInput), 0); //should be a vector
    renderer.updateFrameSamplers(sampledImages, 1); //image vector, binding number

    //add the framebuffers
    renderer.addRenderPassInstance("paint0", "offscreen", "offscreen");
    renderer.addRenderPassInstance("swap", "swapchain", "swap");

    renderer.recordRenderCommands(0, {0, 1});

    const uint8_t paintCommand = 0;
    const uint8_t swapOnlyCmd = 0;
    const uint8_t paletteCmd = 0;
    const uint8_t brushSizeCmd = 0;
    
    uint32_t cmdIdCache{0};
    std::array<UserInput, 10> inputCache;

    FragmentInput fragInput;
    renderer.bindUboData(static_cast<void*>(&fragInput), sizeof(FragmentInput), 0);

    float cMapX = float(WIDTH)/float(C_WIDTH);
    float cMapY = float(HEIGHT)/float(C_HEIGHT);

    fragInput.xform = glm::scale(glm::mat4(1.), glm::vec3(cMapX, cMapY, 1.0));

    glm::mat4 toCanvasSpace = glm::scale(glm::mat4(1.), glm::vec3(cMapX, cMapY, 1.0)); 
    glm::mat4 transMatrix{1.};
    glm::mat4 pivotMatrix = glm::translate(glm::mat4(1.), glm::vec3{-0.5 * cMapX, -0.5 * cMapY, 0.0});
    glm::mat4 scaleRotMatrix{1.};
    fragInput.xform = toCanvasSpace;

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

    InputState state{InputState::Idle};

    for (int i = 0; i < N_FRAMES; i++) 
    {
        auto& input = eventHander.fetchUserInput(true);
        switch (input.eventType)
        {
            case EventType::Motion:
                {
                    glm::vec4 mPos = glm::vec4{
                        input.mouseX / float(WIDTH), input.mouseY / float(HEIGHT), 1., 1.};
                    switch (state)
                    {
                        case InputState::Paint:
                            {
                                mPos = fragInput.xform * mPos;
                                fragInput.mouseX = mPos.x;
                                fragInput.mouseY = mPos.y;
                                myTimer.start();
                                renderer.render(paintCommand, true);
                                myTimer.end("Render");
                                break;
                            }
                        case InputState::Idle:
                            {
                                mPos = fragInput.xform * mPos;
                                fragInput.mouseX = mPos.x;
                                fragInput.mouseY = mPos.y;
                                renderer.render(brushSizeCmd, true);
                                break;
                            }
                        case InputState::Rotate:
                            {
                                angle = 
                                    angleScale * input.mouseX / float(WIDTH) - mouseXCache + 
                                    angleScale * input.mouseY / float(HEIGHT) - mouseYCache;
                                scaleRotMatrix = scaleRotCache * transMatrix * glm::rotate(glm::mat4(1.), angle, {0, 0, 1.}) * glm::inverse(transMatrix);
                                fragInput.xform = 
                                    glm::inverse(pivotMatrix) * 
                                    scaleRotMatrix * 
                                    transMatrix * 
                                    pivotMatrix * 
                                    toCanvasSpace;
                                renderer.render(swapOnlyCmd, true);
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
                                renderer.render(swapOnlyCmd, true);
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
                                renderer.render(swapOnlyCmd, true);
                                break;
                            }
                        case InputState::BrushResize:
                            {
                                float brushSize = 
                                    mPos.x - mouseXCache +
                                    mPos.y - mouseYCache +
                                    brushSizeCache;
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
                    mPos = glm::vec4{
                        input.mouseX / float(WIDTH), 
                        input.mouseY / float(HEIGHT), 
                        0., 1.};
                    switch (input.mouseButton)
                    {
                        case MouseButton::Left:
                            {
                                state = InputState::Paint;
                                mPos = fragInput.xform * mPos;
                                fragInput.mouseX = mPos.x;
                                fragInput.mouseY = mPos.y;
                                renderer.render(paintCommand, true);
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
            case EventType::Keypress:
                {
                    switch (static_cast<PressAction>(input.key))
                    {
                        case PressAction::Rotate:
                            {
                                std::cout << "Entering rotation" << std::endl;
                                state = InputState::Rotate;
                                mouseXCache = angleScale * input.mouseX / float(WIDTH);
                                mouseYCache = angleScale * input.mouseY / float(HEIGHT);
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
                                mouseXCache = input.mouseX / float(WIDTH);
                                mouseYCache = input.mouseY / float(HEIGHT);
                                state = InputState::BrushResize;
                                renderer.render(brushSizeCmd, true);
                                break;
                            }
                        case PressAction::Exit:
                            {
                                std::cout << "Closing application" << std::endl;
                                return 0;
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
            case EventType::MouseRelease:
                {
                    transCache = transMatrix;
                    scaleRotCache = scaleRotMatrix;
                    state = InputState::Idle;
                    break;
                }
            case EventType::LeaveWindow:
                {
                    rotateDown = false;
                    state = InputState::Idle;
                    break;
                }
            case EventType::EnterWindow:
                {
                    break;
                }
        }
    }
}

