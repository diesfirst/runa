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
#include "../thirdparty/lodepng.cpp"

constexpr uint32_t N_FRAMES = 10000000;
constexpr uint32_t WIDTH = 1400;
constexpr uint32_t HEIGHT = 1000;
constexpr uint32_t C_WIDTH = 8000;
constexpr uint32_t C_HEIGHT = 8000;
constexpr double PI = 3.14159265358979323846;

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
    Exit = static_cast<uint8_t>(Key::Esc),
    Save = static_cast<uint8_t>(Key::S)
};

enum class ReleaseAction : uint8_t 
{
    Rotate = static_cast<uint8_t>(Key::Alt)
};

int main(int argc, char *argv[])
{
    const std::string shaderDir{SHADER_DIR};

    Context context;
    XWindow window(WIDTH, HEIGHT);
    window.open();
    EventHandler eventHander(window);

    Renderer renderer(context, window);

    auto& paint0 = renderer.createAttachment("paint0", {C_WIDTH, C_HEIGHT});

    //setup descriptor set for painting pipeline
    std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
    bindings[0].setBinding(0);
    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    bindings[0].setDescriptorCount(1);
    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    bindings[1].setBinding(1);
    bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    bindings[1].setDescriptorCount(1);
    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    auto paintingLayout = renderer.createDescriptorSetLayout("paintLayout", bindings);

    renderer.createFrameDescriptorSets({paintingLayout}); //order matters for access
    
    auto paintPLayout = renderer.createPipelineLayout("paintpipelayout", {paintingLayout});

    auto sampledImages = std::vector<const mm::Image*>{
            &paint0.getImage(0)};
    
    //vert shaders
    auto& quadShader = renderer.loadVertShader(shaderDir + "fullscreen_tri.spv", "vert1");

    //frag shaders
    auto& tarotShader = renderer.loadFragShader(shaderDir + "spot_xform.spv", "frag1");
    auto& swapShader = renderer.loadFragShader(shaderDir + "composite_xform.spv", "swap");
    auto& paletteShader = renderer.loadFragShader(shaderDir + "uv_00.spv", "uv00");
    tarotShader.setWindowResolution(C_WIDTH, C_HEIGHT);
    swapShader.setWindowResolution(WIDTH, HEIGHT); 
    paletteShader.setWindowResolution(WIDTH, HEIGHT);
    swapShader.specData.integer0 = 0; 
    swapShader.specData.integer1 = 0; 
    
    //we should actually not be doing this. the attachment size make sense for a spec constant
    //but the array indices would be better handled by push constants

    //create render passes
    auto& offScreenLoadPass = renderer.createRenderPass("offscreen");
    renderer.prepareAsOffscreenPass(offScreenLoadPass); //can make this a static function of RenderPass
    auto& palettePass = renderer.createRenderPass("palette");
    renderer.prepareAsOffscreenPass(palettePass);
    auto& swapchainPass = renderer.createRenderPass("swapchain");
    renderer.prepareAsSwapchainPass(swapchainPass); //can make this a static function of RenderPass
    
    //create pipelines
    vk::Rect2D swapRenderArea{{0,0}, {WIDTH, HEIGHT}};
    vk::Rect2D offscreenRenderArea{{0,0}, {C_WIDTH, C_HEIGHT}};
    auto& offscreenPipe = renderer.createGraphicsPipeline(
            "offscreen", paintPLayout, quadShader, tarotShader, offScreenLoadPass, offscreenRenderArea, false);
    auto& swapchainPipe = renderer.createGraphicsPipeline(
            "swap", paintPLayout, quadShader, swapShader, swapchainPass, swapRenderArea, false);

    offscreenPipe.create();
    swapchainPipe.create();
    
    //ubo stuff must update the sets before recording the command buffers

    renderer.initFrameUBOs(sizeof(FragmentInput), 0); //should be a vector
    renderer.updateFrameSamplers(sampledImages, 1); //image vector, binding number

    //add the framebuffers
    renderer.addFramebuffer(paint0, offScreenLoadPass, offscreenPipe);
    renderer.addFramebuffer(TargetType::swapchain, swapchainPass, swapchainPipe);

    renderer.recordRenderCommands(0, {0, 1});
    renderer.recordRenderCommands(1, {1});

    const int swapOnlyCmd = 1;
    
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
    glm::vec4 mPos{0};
    glm::vec4 transMouseCache{0};

    for (int i = 0; i < N_FRAMES; i++) 
    {
        auto& input = eventHander.fetchUserInput(true);
        if (input.lmButtonDown)
        {
            glm::vec4 mPos = glm::vec4{
                input.mouseX / float(WIDTH), input.mouseY / float(HEIGHT), 1., 1.};
            mPos = fragInput.xform * mPos;
            fragInput.mouseX = mPos.x;
            fragInput.mouseY = mPos.y;
            myTimer.start();
            renderer.render(input.cmdId, true);
            myTimer.end("Render");
        }
        if (input.rmButtonDown)
        {
            if (input.eventType == EventType::MousePress)
            {
                mouseXCache = (float)input.mouseX / (float)WIDTH;
                mouseYCache = (float)input.mouseY / (float)HEIGHT;
            }
            scale = 
                (float)input.mouseX / (float)WIDTH - mouseXCache +
                (float)input.mouseY / (float)HEIGHT - mouseYCache +
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
        }
        if (input.mmButtonDown)
        {
            mPos = glm::vec4{
                input.mouseX / float(WIDTH), input.mouseY / float(HEIGHT), 0., 1.};
            mPos = toCanvasSpace * mPos;
            if (input.eventType == EventType::MousePress)
            {
                transMouseCache = mPos;
            }
            mPos = mPos - transMouseCache;
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
        }
        if (rotateDown)
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
        }
        if (input.eventType == EventType::Keypress)
        {
            switch (static_cast<PressAction>(input.key))
            {
                case PressAction::Rotate:
                    {
                        std::cout << "Entering rotation" << std::endl;
                        rotateDown = true;
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
                        std::cout << "R: " << static_cast<uint16_t>(*ptr) << std::endl;
                        fragInput.r = float(*ptr++) / 255.;
                        std::cout << "G: " << static_cast<uint16_t>(*ptr) << std::endl;
                        fragInput.g = float(*ptr++) / 255.;
                        std::cout << "B: " << static_cast<uint16_t>(*ptr) << std::endl;
                        fragInput.b = float(*ptr++) / 255.;
                        renderer.popBufferBlock();
                        renderer.render(swapOnlyCmd, true);
                        break;
                    }
                case PressAction::BrushSize:
                    {
                        std::cout << "Choose brush size" << std::endl;
                        std::cin >> fragInput.brushSize;
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
                        auto block = renderer.copyAttachmentToHost("paint0", region);
                        uint8_t* ptr = static_cast<uint8_t*>(block->pHostMemory);
                        std::cout << "Printing block values" << std::endl;
                        for (int i = 0; i < block->size / 4; i++) 
                        {
                            std::cout << "R: " << static_cast<uint16_t>(*ptr) << std::endl;   
                            ptr += 4;
                        }
                        std::cout << "success" << std::endl;
                        renderer.render(swapOnlyCmd, true);
                        renderer.popBufferBlock();
                    }
            }
        }
        if (input.eventType == EventType::Keyrelease)
        {
            switch (static_cast<ReleaseAction>(input.key))
            {
                case ReleaseAction::Rotate:
                    {
                        std::cout << "Leaving rotation" << std::endl;
                        rotateDown = false;
                        scaleRotCache = scaleRotMatrix;
                        break;
                    }
            }
        }
        if (input.eventType == EventType::MouseRelease)
        {
            transCache = transMatrix;
            scaleRotCache = scaleRotMatrix;
        }
        if (input.eventType == EventType::LeaveWindow)
        {
            rotateDown = false;
        }
    }

}

