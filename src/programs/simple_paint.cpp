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

constexpr char SHADER_DIR[] = "build/shaders/";
Timer myTimer;

struct FragmentInput
{
	float time{0};
	float mouseX{0};
	float mouseY{0};	
	int blur{0};
    float r{1.};
    float g{1.};
    float b{1.};
    float a{1.};
    float brushSize{1.};
    int layerId{0};
};

int main(int argc, char *argv[])
{
    const std::string shaderDir{SHADER_DIR};

    Context context;
    XWindow window(WIDTH, HEIGHT);
    window.open();
    EventHandler eventHander(window);

    Renderer renderer(context, window);

    auto& paint0 = renderer.createAttachment("paint0", {WIDTH, HEIGHT});
    auto& paint1 = renderer.createAttachment("paint1", {WIDTH, HEIGHT});
    auto& paint2 = renderer.createAttachment("paint2", {WIDTH, HEIGHT});
    auto& background = renderer.createAttachment("background", {WIDTH, HEIGHT});
    auto& foreground = renderer.createAttachment("foreground", {WIDTH, HEIGHT});

    //setup descriptor set for painting pipeline
    std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
    bindings[0].setBinding(0);
    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    bindings[0].setDescriptorCount(1);
    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    bindings[1].setBinding(1);
    bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    bindings[1].setDescriptorCount(5);
    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    auto paintingLayout = renderer.createDescriptorSetLayout("paintLayout", bindings);

    renderer.createFrameDescriptorSets({paintingLayout}); //order matters for access
    
    auto paintPLayout = renderer.createPipelineLayout("paintpipelayout", {paintingLayout});

    auto sampledImages = std::vector<const mm::Image*>{
            &paint0.getImage(0), 
            &paint1.getImage(0), 
            &paint2.getImage(0), 
            &background.getImage(0),
            &foreground.getImage(0)};
    
    //vert shaders
    auto& quadShader = renderer.loadVertShader(shaderDir + "fullscreen_tri.spv", "vert1");

    //frag shaders
    auto& tarotShader = renderer.loadFragShader(shaderDir + "spot.spv", "frag1");
    auto& fgShader = renderer.loadFragShader(shaderDir + "composite.spv", "fg");
    auto& bgShader = renderer.loadFragShader(shaderDir + "composite.spv", "bg");
    auto& swapShader = renderer.loadFragShader(shaderDir + "composite.spv", "swap");
    tarotShader.setWindowResolution(WIDTH, HEIGHT);
    fgShader.setWindowResolution(WIDTH, HEIGHT);
    fgShader.specData.integer0 = 0; //array limit
    fgShader.specData.integer1 = 2; //array limit
    bgShader.setWindowResolution(WIDTH, HEIGHT);
    bgShader.specData.integer0 = 4; //
    bgShader.specData.integer1 = 4; //should simply copy the result of the fg shader to the bg
    swapShader.setWindowResolution(WIDTH, HEIGHT); 
    swapShader.specData.integer0 = 3; 
    swapShader.specData.integer1 = 4; 
    
    //we should actually not be doing this. the attachment size make sense for a spec constant
    //but the array indices would be better handled by push constants

    //create render passes
    auto& offScreenLoadPass = renderer.createRenderPass("offscreen", true);
    renderer.prepareAsOffscreenPass(offScreenLoadPass); //can make this a static function of RenderPass
    auto& swapchainPass = renderer.createRenderPass("swapchain", false); 
    renderer.prepareAsSwapchainPass(swapchainPass); //can make this a static function of RenderPass

    auto& offscreenClearPass = renderer.createRenderPass("composite", true);

    vk::ClearColorValue cv;
    cv.setFloat32({.0,.0,.0,0.});
	offscreenClearPass.createColorAttachment(
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eShaderReadOnlyOptimal,
            cv,
            vk::AttachmentLoadOp::eClear);
	vk::SubpassDependency d0, d1;
	d0.setSrcSubpass(VK_SUBPASS_EXTERNAL);
	d0.setDstSubpass(0);
	d0.setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader);
	d0.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	d0.setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
	d0.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
	d0.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	d1.setSrcSubpass(0);
	d1.setDstSubpass(VK_SUBPASS_EXTERNAL);
	d1.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	d1.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);
	d1.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
	d1.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
	d1.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	offscreenClearPass.addSubpassDependency(d0);
	offscreenClearPass.addSubpassDependency(d1);
	offscreenClearPass.createSubpass();
	offscreenClearPass.create();
    
    //create pipelines
    auto& offscreenPipe = renderer.createGraphicsPipeline(
            "offscreen", paintPLayout, quadShader, tarotShader, offScreenLoadPass, false);
    auto& offscreenPipeErase = renderer.createGraphicsPipeline(
            "offscreenErase", paintPLayout, quadShader, tarotShader, offScreenLoadPass, false);
    auto& foregroundPipe = renderer.createGraphicsPipeline(
            "foreground", paintPLayout, quadShader, fgShader, offscreenClearPass, false);
    auto& backgroundPipe = renderer.createGraphicsPipeline(
            "background", paintPLayout, quadShader, bgShader, offScreenLoadPass, false);
    auto& swapchainPipe = renderer.createGraphicsPipeline(
            "swap", paintPLayout, quadShader, swapShader, swapchainPass, false);

    offscreenPipeErase.attachmentStates.at(0).setColorBlendOp(vk::BlendOp::eMinusClampedEXT);
    offscreenPipeErase.attachmentStates.at(0).setAlphaBlendOp(vk::BlendOp::eMinusClampedEXT);

    offscreenPipe.create();
    offscreenPipeErase.create();
    foregroundPipe.create();
    backgroundPipe.create();
    swapchainPipe.create();

    //add the framebuffers
    renderer.addFramebuffer(paint0, offScreenLoadPass, offscreenPipe);
    renderer.addFramebuffer(paint1, offScreenLoadPass, offscreenPipe);
    renderer.addFramebuffer(paint2, offScreenLoadPass, offscreenPipe);
    renderer.addFramebuffer(paint0, offscreenClearPass, offscreenPipe);
    renderer.addFramebuffer(paint1, offscreenClearPass, offscreenPipe);
    renderer.addFramebuffer(paint2, offscreenClearPass, offscreenPipe);
    renderer.addFramebuffer(foreground, offscreenClearPass, foregroundPipe);
    renderer.addFramebuffer(background, offScreenLoadPass, backgroundPipe);
    renderer.addFramebuffer(TargetType::swapchain, swapchainPass, swapchainPipe);
    renderer.addFramebuffer(paint0, offScreenLoadPass, offscreenPipeErase);
    renderer.addFramebuffer(paint1, offScreenLoadPass, offscreenPipeErase);
    renderer.addFramebuffer(paint2, offScreenLoadPass, offscreenPipeErase);
   
    
    //ubo stuff must update the sets before recording the command buffers

    renderer.initFrameUBOs(sizeof(FragmentInput), 0); //should be a vector
    renderer.updateFrameSamplers(sampledImages, 1); //image vector, binding number

    renderer.recordRenderCommands(0, {0, 6, 8});
    renderer.recordRenderCommands(1, {1, 6, 8}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(2, {2, 6, 8}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(3, {3, 6, 8}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(4, {4, 6, 8}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(5, {5, 6, 8}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(6, {7, 8}); //copy to background whatever is in the foreground
    renderer.recordRenderCommands(7, {9, 6, 8});
    renderer.recordRenderCommands(8, {10, 6, 8});
    renderer.recordRenderCommands(9, {11, 6, 8});
    
    uint32_t cmdIdCache{0};
    std::array<UserInput, 10> inputCache;

    //single renderes to get the attachements in the right format
    renderer.render(0, false);
    renderer.render(1, false); 
    renderer.render(2, false);
    renderer.render(3, false);
    renderer.render(6, false);

    FragmentInput fragInput;
    renderer.bindUboData(static_cast<void*>(&fragInput), sizeof(FragmentInput), 0);

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N_FRAMES; i++) 
    {
        auto& input = eventHander.fetchUserInput(true);
        if (input.cmdId != cmdIdCache) //changing layers
        {
            //write to the cache on the last layer
            auto& cacheLeaving = inputCache.at(cmdIdCache); 
            cacheLeaving = input;

            //read from the cache of the layer we're entering
            auto& cacheEntering = inputCache.at(input.cmdId);
            input.r = cacheEntering.r;
            input.g = cacheEntering.g;
            input.b = cacheEntering.b;
            input.a = cacheEntering.a;
            input.brushSize = cacheEntering.brushSize;

            auto t1 = std::chrono::high_resolution_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(t1 - t0).count();
            fragInput = {
                    elapsedTime,
                    (float)0,
                    (float)0, 
                    input.blur,
                    input.r,
                    input.g,
                    input.b,
                    input.a,
                    input.brushSize,
                    (int)input.cmdId};
            renderer.render(input.cmdId, true);
            cmdIdCache = input.cmdId;
        }
        if (input.mButtonDown)
        {
            auto t1 = std::chrono::high_resolution_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(t1 - t0).count();
            fragInput = {
                    elapsedTime,
                    (float)input.mouseX,
                    (float)input.mouseY, 
                    input.blur,
                    input.r,
                    input.g,
                    input.b,
                    input.a,
                    input.brushSize,
                    (int)input.cmdId};
            renderer.render(input.cmdId, true);
        }
    }

    sleep(4);
}

