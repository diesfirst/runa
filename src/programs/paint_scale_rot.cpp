#include "../core/window.hpp"
#include "../core/renderer.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "../core/util.hpp"
#include <unistd.h>
#include "../core/event.hpp"

constexpr uint32_t N_FRAMES = 10000000;
constexpr uint32_t WIDTH = 666;
constexpr uint32_t HEIGHT = 888;
constexpr uint32_t C_WIDTH = 4000;
constexpr uint32_t C_HEIGHT = 4000;

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
    float sx{1.};
    float sy{1.};
    float tx{1.};
    float ty{1.};
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
    tarotShader.setWindowResolution(C_WIDTH, C_HEIGHT);
    swapShader.setWindowResolution(WIDTH, HEIGHT); 
    swapShader.specData.integer0 = 0; 
    swapShader.specData.integer1 = 0; 
    
    //we should actually not be doing this. the attachment size make sense for a spec constant
    //but the array indices would be better handled by push constants

    //create render passes
    auto& offScreenLoadPass = renderer.createRenderPass("offscreen", true);
    renderer.prepareAsOffscreenPass(offScreenLoadPass); //can make this a static function of RenderPass
    auto& swapchainPass = renderer.createRenderPass("swapchain", false); 
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
    
    uint32_t cmdIdCache{0};
    std::array<UserInput, 10> inputCache;

    //single renderes to get the attachements in the right format
//    renderer.render(0, false);

    FragmentInput fragInput;
    renderer.bindUboData(static_cast<void*>(&fragInput), sizeof(FragmentInput), 0);

    float cMapX = float(WIDTH)/float(C_WIDTH);
    float cMapY = float(HEIGHT)/float(C_HEIGHT);

    fragInput.sx = cMapX;
    fragInput.sy = cMapY;

    float scaleCache;
    float txCache;
    float tyCache;

    for (int i = 0; i < N_FRAMES; i++) 
    {
        auto& input = eventHander.fetchUserInput(true);
        if (input.lmButtonDown)
        {
            float mx = (float)input.mouseX / (float)C_WIDTH;
            float my = (float)input.mouseY / (float)C_HEIGHT; 
            fragInput.mouseX = mx;
            fragInput.mouseY = my;
            fragInput.r = input.r;
            fragInput.g = input.g;
            fragInput.b = input.b;
            fragInput.a = input.a;
            fragInput.brushSize = input.brushSize;
            std::cout << "mx " << mx << std::endl;
            std::cout << "my " << my << std::endl;
            myTimer.start();
            renderer.render(input.cmdId, true);
            myTimer.end("Render");
        }
        if (input.rmButtonDown)
        {
            float scale = (float)input.mouseX / (float)WIDTH + scaleCache;
            fragInput.sx = cMapX * scale;
            fragInput.sy = cMapY * scale;
            renderer.render(input.cmdId, true);
        }
    }

    sleep(4);
}

