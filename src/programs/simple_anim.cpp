#include "../core/window.hpp"
#include "../core/renderer.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "../core/util.hpp"
#include <unistd.h>
#include "../core/event.hpp"

constexpr uint32_t N_FRAMES = 10000000;
constexpr char SHADER_DIR[] = "build/shaders/";
Timer myTimer;

int main(int argc, char *argv[])
{
    const std::string shaderDir{SHADER_DIR};

    Context context;
    XWindow window(666, 999);
    window.open();
    EventHandler eventHander(window);

    Renderer renderer(context, window);

    auto& paint0 = renderer.createAttachment("paint0", {666, 999});
    auto& paint1 = renderer.createAttachment("paint1", {666, 999});
    auto& paint2 = renderer.createAttachment("paint2", {666, 999});
    auto& paint3 = renderer.createAttachment("paint3", {666, 999});
    auto& paint4 = renderer.createAttachment("paint4", {666, 999});
    auto& paint5 = renderer.createAttachment("paint5", {666, 999});
    auto& paint6 = renderer.createAttachment("paint6", {666, 999});

    //setup descriptor set for painting pipeline
    std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
    bindings[0].setBinding(0);
    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    bindings[0].setDescriptorCount(1);
    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    bindings[1].setBinding(1);
    bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    bindings[1].setDescriptorCount(7);
    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    auto paintingLayout = renderer.createDescriptorSetLayout("paintLayout", bindings);

    renderer.createFrameDescriptorSets({paintingLayout}); //order matters for access
    
    auto paintPLayout = renderer.createPipelineLayout("paintpipelayout", {paintingLayout});

    auto images = std::vector<const mm::Image*>{
            &paint0.getImage(0), 
            &paint1.getImage(0), 
            &paint2.getImage(0), 
            &paint3.getImage(0),
            &paint4.getImage(0),
            &paint5.getImage(0),
            &paint6.getImage(0)};

    renderer.initFrameUBOs(0); //should be a vector
    renderer.updateFrameSamplers(images, 1); //image vector, binding number

    auto& quadShader = renderer.loadVertShader(shaderDir + "fullscreen_tri.spv", "vert1");
    auto& tarotShader = renderer.loadFragShader(shaderDir + "spot.spv", "frag1");
    auto& passShader = renderer.loadFragShader(shaderDir + "layer_select.spv", "osFrag");

    //create render passes
    auto& offScreenPass = renderer.createRenderPass("offscreen", true);
    renderer.prepareAsOffscreenPass(offScreenPass); //can make this a static function of RenderPass
    auto& swapchainPass = renderer.createRenderPass("swapchain", false); 
    renderer.prepareAsSwapchainPass(swapchainPass); //can make this a static function of RenderPass

    auto& compositePass = renderer.createRenderPass("composite", true);

    vk::ClearColorValue cv;
    cv.setFloat32({.0,.0,.0,0.});
	compositePass.createColorAttachment(
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

	compositePass.addSubpassDependency(d0);
	compositePass.addSubpassDependency(d1);
	compositePass.createSubpass();
	compositePass.create();
    
    //create pipelines
    auto& offscreenPipe = renderer.createGraphicsPipeline(
            "offscreen", paintPLayout, quadShader, tarotShader, offScreenPass, false);
    auto& swapchainPipe = renderer.createGraphicsPipeline(
            "default", paintPLayout, quadShader, passShader, swapchainPass, false);

    //add the framebuffers
    renderer.addFramebuffer(paint0, offScreenPass, offscreenPipe);
    renderer.addFramebuffer(paint1, offScreenPass, offscreenPipe);
    renderer.addFramebuffer(paint2, offScreenPass, offscreenPipe);
    renderer.addFramebuffer(paint3, offScreenPass, offscreenPipe);
    renderer.addFramebuffer(paint4, offScreenPass, offscreenPipe);
    renderer.addFramebuffer(paint5, offScreenPass, offscreenPipe);
    renderer.addFramebuffer(paint6, offScreenPass, offscreenPipe);
    renderer.addFramebuffer(TargetType::swapchain, swapchainPass, swapchainPipe);

    renderer.recordRenderCommands(0, {0, 7});
    renderer.recordRenderCommands(1, {1, 7}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(2, {2, 7}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(3, {3, 7}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(4, {4, 7}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(5, {5, 7}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(6, {6, 7}); //(commandBuffer id, {fbids})
    

    //single renderes to get the attachements in the right format
    renderer.render(0);
    renderer.render(1); 
    renderer.render(2);
    renderer.render(3);
    renderer.render(4);
    renderer.render(5);
    renderer.render(6);

    uint32_t cmdIdCache{0};
    std::array<UserInput, 7> inputCache;

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
            renderer.setFragmentInput(0, {
                    elapsedTime,
                    (float)0,
                    (float)0, 
                    input.blur,
                    input.r,
                    input.g,
                    input.b,
                    input.a,
                    input.brushSize,
                    (int)input.cmdId});
            myTimer.start();
            renderer.render(input.cmdId);
            myTimer.end("Render submission");
            cmdIdCache = input.cmdId;
        }
        if (input.mButtonDown)
        {
            auto t1 = std::chrono::high_resolution_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(t1 - t0).count();
            renderer.setFragmentInput(0, {
                    elapsedTime,
                    (float)input.mouseX,
                    (float)input.mouseY, 
                    input.blur,
                    input.r,
                    input.g,
                    input.b,
                    input.a,
                    input.brushSize,
                    (int)input.cmdId});
            myTimer.start();
            renderer.render(input.cmdId);
            myTimer.end("Render submission");
        }
    }

    sleep(4);
}

