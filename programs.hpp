#include "window.hpp"
#include "renderer.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "util.hpp"
#include <unistd.h>
#include "event.hpp"

constexpr uint32_t N_FRAMES = 10000000;
Timer myTimer;



//void program0(const std::string card_name)
//{
//	const std::string path{"shaders/tarot/" + card_name + ".spv"};
//
//	Context context;
//	XWindow window(666, 999);
//	window.open();
//	EventHandler eventHander(window);
//
//	Renderer renderer(context, window);
//
//    auto& background = renderer.createAttachment("background", {666, 999});
//    auto& foreground = renderer.createAttachment("foreground", {666, 999});
//    auto& paint0 = renderer.createAttachment("paint0", {666, 999});
//
//    //setup descriptor set for painting pipeline
//    std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
//    bindings[0].setBinding(0);
//    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
//    bindings[0].setDescriptorCount(1);
//    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);
//
//    bindings[1].setBinding(1);
//    bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
//    bindings[1].setDescriptorCount(1);
//    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);
//
//    auto paintingLayout = renderer.createDescriptorSetLayout("paintLayout", bindings);
//
//    bindings[1].setDescriptorType(vk::DescriptorType::eSampler);
//
//    bindings.resize(3);
//    bindings[2].setBinding(2);
//    bindings[2].setDescriptorType(vk::DescriptorType::eSampledImage);
//    bindings[2].setDescriptorCount(50); //arbitrary big number
//    bindings[2].setStageFlags(vk::ShaderStageFlagBits::eFragment);
//
//    auto mergeLayout = renderer.createDescriptorSetLayout("mergeLayout", bindings);
//
//    renderer.createFrameDescriptorSets({paintingLayout}); //order matters for access
//    renderer.createOwnDescriptorSets({mergeLayout});
//    
//    auto paintPLayout = renderer.createPipelineLayout("paintpipelayout", {paintingLayout});
//    auto mergePLayout = renderer.createPipelineLayout("mergepipelayout", {mergeLayout});
//
//    auto& image = paint0.getImage(0);
//    renderer.initFrameUBOs(0); //should be a vector
//    renderer.updateFrameSamplers(&image.getView(), &image.getSampler(), 1);
//
//	auto& quadShader = renderer.loadVertShader("shaders/tarot/fullscreen_tri.spv", "vert1");
//	auto& tarotShader = renderer.loadFragShader(path, "frag1");
//	auto& blurShader = renderer.loadFragShader("shaders/radialBlur.spv", "osFrag");
//
//    //create render passes
//    auto& offScreenPass = renderer.createRenderPass("offscreen", true);
//    renderer.prepareAsOffscreenPass(offScreenPass); //can make this a static function of RenderPass
//    auto& swapchainPass = renderer.createRenderPass("swapchain", false); 
//    renderer.prepareAsSwapchainPass(swapchainPass); //can make this a static function of RenderPass
//    
//    //create pipelines
//	auto& offscreenPipe = renderer.createGraphicsPipeline(
//            "offscreen", paintPLayout, quadShader, tarotShader, offScreenPass, false);
//	auto& swapchainPipe = renderer.createGraphicsPipeline(
//            "default", paintPLayout, quadShader, blurShader, swapchainPass, false);
//
//    //add the framebuffers
//    renderer.addFramebuffer(paint0, offScreenPass, offscreenPipe);
//    renderer.addFramebuffer(TargetType::swapchain, swapchainPass, swapchainPipe);
//
//    renderer.recordRenderCommands(0, {0, 1});
//
//    renderer.render(0);
//
//    auto t0 = std::chrono::high_resolution_clock::now();
//	for (int i = 0; i < N_FRAMES; i++) 
//	{
//		const auto input = eventHander.fetchUserInput(true);
//        if (input.mButtonDown)
//        {
//            auto t1 = std::chrono::high_resolution_clock::now();
//            auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(t1 - t0).count();
//            renderer.setFragmentInput(0, {
//                    elapsedTime,
//                    (float)input.mouseX,
//                    (float)input.mouseY, 
//                    input.blur,
//                    input.r,
//                    input.g,
//                    input.b});
//            myTimer.start();
//            renderer.render(0);
//            myTimer.end("Render submission");
//        }
//	}
//
//    sleep(4);
//}
//
//void program1(const std::string card_name)
//{
//	const std::string path{"shaders/tarot/" + card_name + ".spv"};
//
//	Context context;
//	XWindow window(666, 999);
//	window.open();
//	EventHandler eventHander(window);
//
//	Renderer renderer(context, window);
//
//    auto& paint0 = renderer.createAttachment("paint0", {666, 999});
//    auto& paint1 = renderer.createAttachment("paint1", {666, 999});
//
//    //setup descriptor set for painting pipeline
//    std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
//    bindings[0].setBinding(0);
//    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
//    bindings[0].setDescriptorCount(1);
//    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);
//
//    bindings[1].setBinding(1);
//    bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
//    bindings[1].setDescriptorCount(2);
//    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);
//
//    auto paintingLayout = renderer.createDescriptorSetLayout("paintLayout", bindings);
//
//    renderer.createFrameDescriptorSets({paintingLayout}); //order matters for access
//    
//    auto paintPLayout = renderer.createPipelineLayout("paintpipelayout", {paintingLayout});
//
//    auto images = std::vector<const mm::Image*>{
//        &paint0.getImage(0), &paint1.getImage(0)};
//
//
//    renderer.initFrameUBOs(0); //should be a vector
//    renderer.updateFrameSamplers(images, 1);
//
//	auto& quadShader = renderer.loadVertShader("shaders/tarot/fullscreen_tri.spv", "vert1");
//	auto& tarotShader = renderer.loadFragShader(path, "frag1");
//	auto& blurShader = renderer.loadFragShader("shaders/radialBlur.spv", "osFrag");
//
//    //create render passes
//    auto& offScreenPass = renderer.createRenderPass("offscreen", true);
//    renderer.prepareAsOffscreenPass(offScreenPass); //can make this a static function of RenderPass
//    auto& swapchainPass = renderer.createRenderPass("swapchain", false); 
//    renderer.prepareAsSwapchainPass(swapchainPass); //can make this a static function of RenderPass
//    
//    //create pipelines
//	auto& offscreenPipe = renderer.createGraphicsPipeline(
//            "offscreen", paintPLayout, quadShader, tarotShader, offScreenPass, false);
//	auto& swapchainPipe = renderer.createGraphicsPipeline(
//            "default", paintPLayout, quadShader, blurShader, swapchainPass, false);
//
//    //add the framebuffers
//    renderer.addFramebuffer(paint0, offScreenPass, offscreenPipe);
//    renderer.addFramebuffer(TargetType::swapchain, swapchainPass, swapchainPipe);
//    renderer.addFramebuffer(paint1, offScreenPass, offscreenPipe);
//
//    renderer.recordRenderCommands(0, {0, 1});
//    renderer.recordRenderCommands(1, {2, 1});
//
//    renderer.render(0);
//    renderer.render(1); //single renderes to get the attachements in the right format
//
//    auto t0 = std::chrono::high_resolution_clock::now();
//	for (int i = 0; i < N_FRAMES; i++) 
//	{
//		const auto input = eventHander.fetchUserInput(true);
//        if (input.mButtonDown)
//        {
//            auto t1 = std::chrono::high_resolution_clock::now();
//            auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(t1 - t0).count();
//            renderer.setFragmentInput(0, {
//                    elapsedTime,
//                    (float)input.mouseX,
//                    (float)input.mouseY, 
//                    input.blur,
//                    input.r,
//                    input.g,
//                    input.b,
//                    (int)input.cmdId});
//            myTimer.start();
//            renderer.render(input.cmdId);
//            myTimer.end("Render submission");
//        }
//	}
//
//    sleep(4);
//}

//void program2(const std::string card_name)
//{
//	const std::string path{"shaders/tarot/" + card_name + ".spv"};
//
//	Context context;
//	XWindow window(666, 999);
//	window.open();
//	EventHandler eventHander(window);
//
//	Renderer renderer(context, window);
//
//    auto& paint0 = renderer.createAttachment("paint0", {666, 999});
//    auto& paint1 = renderer.createAttachment("paint1", {666, 999});
//    auto& paint2 = renderer.createAttachment("paint2", {666, 999});
//    auto& paint3 = renderer.createAttachment("paint3", {666, 999});
//
//    //setup descriptor set for painting pipeline
//    std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
//    bindings[0].setBinding(0);
//    bindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
//    bindings[0].setDescriptorCount(1);
//    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);
//
//    bindings[1].setBinding(1);
//    bindings[1].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
//    bindings[1].setDescriptorCount(4);
//    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eFragment);
//
//    auto paintingLayout = renderer.createDescriptorSetLayout("paintLayout", bindings);
//
//    renderer.createFrameDescriptorSets({paintingLayout}); //order matters for access
//    
//    auto paintPLayout = renderer.createPipelineLayout("paintpipelayout", {paintingLayout});
//
//    auto images = std::vector<const mm::Image*>{
//            &paint0.getImage(0), 
//            &paint1.getImage(0), 
//            &paint2.getImage(0), 
//            &paint3.getImage(0)};
//
//    renderer.initFrameUBOs(0); //should be a vector
//    renderer.updateFrameSamplers(images, 1); //image vector, binding number
//
//	auto& quadShader = renderer.loadVertShader("shaders/tarot/fullscreen_tri.spv", "vert1");
//	auto& tarotShader = renderer.loadFragShader(path, "frag1");
//	auto& blurShader = renderer.loadFragShader("shaders/radialBlur.spv", "osFrag");
//
//    //create render passes
//    auto& offScreenPass = renderer.createRenderPass("offscreen", true);
//    renderer.prepareAsOffscreenPass(offScreenPass); //can make this a static function of RenderPass
//    auto& swapchainPass = renderer.createRenderPass("swapchain", false); 
//    renderer.prepareAsSwapchainPass(swapchainPass); //can make this a static function of RenderPass
//    
//    //create pipelines
//	auto& offscreenPipe = renderer.createGraphicsPipeline(
//            "offscreen", paintPLayout, quadShader, tarotShader, offScreenPass, false);
//	auto& swapchainPipe = renderer.createGraphicsPipeline(
//            "default", paintPLayout, quadShader, blurShader, swapchainPass, false);
//
//    //add the framebuffers
//    renderer.addFramebuffer(paint0, offScreenPass, offscreenPipe);
//    renderer.addFramebuffer(paint1, offScreenPass, offscreenPipe);
//    renderer.addFramebuffer(paint2, offScreenPass, offscreenPipe);
//    renderer.addFramebuffer(paint3, offScreenPass, offscreenPipe);
//    renderer.addFramebuffer(TargetType::swapchain, swapchainPass, swapchainPipe);
//
//    renderer.recordRenderCommands(0, {0, 4});
//    renderer.recordRenderCommands(1, {1, 4}); //(commandBuffer id, {fbids})
//    renderer.recordRenderCommands(2, {2, 4}); //(commandBuffer id, {fbids})
//    renderer.recordRenderCommands(3, {3, 4}); //(commandBuffer id, {fbids})
//    
//
//    //single renderes to get the attachements in the right format
//    renderer.render(0);
//    renderer.render(1); 
//    renderer.render(2);
//    renderer.render(3);
//
//    uint32_t cmdIdCache{0};
//    auto t0 = std::chrono::high_resolution_clock::now();
//
//	for (int i = 0; i < N_FRAMES; i++) 
//	{
//		const auto input = eventHander.fetchUserInput(true);
//        if (input.cmdId != cmdIdCache)
//        {
//            auto t1 = std::chrono::high_resolution_clock::now();
//            auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(t1 - t0).count();
//            renderer.setFragmentInput(0, {
//                    elapsedTime,
//                    (float)0,
//                    (float)0, 
//                    input.blur,
//                    input.r,
//                    input.g,
//                    input.b,
//                    (int)input.cmdId});
//            myTimer.start();
//            renderer.render(input.cmdId);
//            myTimer.end("Render submission");
//            cmdIdCache = input.cmdId;
//        }
//        if (input.mButtonDown)
//        {
//            auto t1 = std::chrono::high_resolution_clock::now();
//            auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(t1 - t0).count();
//            renderer.setFragmentInput(0, {
//                    elapsedTime,
//                    (float)input.mouseX,
//                    (float)input.mouseY, 
//                    input.blur,
//                    input.r,
//                    input.g,
//                    input.b,
//                    (int)input.cmdId});
//            myTimer.start();
//            renderer.render(input.cmdId);
//            myTimer.end("Render submission");
//        }
//	}
//
//    sleep(4);
//}

void program3(const std::string card_name)
{
    const std::string path{"shaders/tarot/" + card_name + ".spv"};

    Context context;
    XWindow window(666, 999);
    window.open();
    EventHandler eventHander(window);

    Renderer renderer(context, window);

    auto& paint0 = renderer.createAttachment("paint0", {666, 999});
    auto& paint1 = renderer.createAttachment("paint1", {666, 999});
    auto& paint2 = renderer.createAttachment("paint2", {666, 999});
    auto& paint3 = renderer.createAttachment("paint3", {666, 999});
    auto& background = renderer.createAttachment("background", {666, 999});

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

    auto images = std::vector<const mm::Image*>{
            &paint0.getImage(0), 
            &paint1.getImage(0), 
            &paint2.getImage(0), 
            &paint3.getImage(0),
            &background.getImage(0)};

    renderer.initFrameUBOs(0); //should be a vector
    renderer.updateFrameSamplers(images, 1); //image vector, binding number

    auto& quadShader = renderer.loadVertShader("shaders/tarot/fullscreen_tri.spv", "vert1");
    auto& tarotShader = renderer.loadFragShader(path, "frag1");
    auto& passShader = renderer.loadFragShader("shaders/passthrough.spv", "osFrag");
    auto& compShader = renderer.loadFragShader("shaders/composite.spv", "comp");

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
    auto& compositePipe = renderer.createGraphicsPipeline(
            "composite", paintPLayout, quadShader, compShader, compositePass, false);
    auto& swapchainPipe = renderer.createGraphicsPipeline(
            "default", paintPLayout, quadShader, passShader, swapchainPass, false);

    //add the framebuffers
    renderer.addFramebuffer(paint0, offScreenPass, offscreenPipe);
    renderer.addFramebuffer(paint1, offScreenPass, offscreenPipe);
    renderer.addFramebuffer(paint2, offScreenPass, offscreenPipe);
    renderer.addFramebuffer(paint3, offScreenPass, offscreenPipe);
    renderer.addFramebuffer(background, compositePass, compositePipe);
    renderer.addFramebuffer(TargetType::swapchain, swapchainPass, swapchainPipe);

    renderer.recordRenderCommands(0, {0, 4, 5});
    renderer.recordRenderCommands(1, {1, 4, 5}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(2, {2, 4, 5}); //(commandBuffer id, {fbids})
    renderer.recordRenderCommands(3, {3, 4, 5}); //(commandBuffer id, {fbids})
    

    //single renderes to get the attachements in the right format
    renderer.render(0);
    renderer.render(1); 
    renderer.render(2);
    renderer.render(3);

    uint32_t cmdIdCache{0};
    std::array<UserInput, 4> inputCache;

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
                    (int)input.cmdId});
            myTimer.start();
            renderer.render(input.cmdId);
            myTimer.end("Render submission");
        }
    }

    sleep(4);
}

