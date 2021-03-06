#include "vulkan/vulkan.hpp"
#include <render/context.hpp>
#include <render/resource.hpp>
#include <render/swapchain.hpp>
#include <render/renderframe.hpp>
#include <render/renderlayer.hpp>
#include <render/attachment.hpp>
#include <render/renderer.hpp>
#include <util/debug.hpp>

namespace sword
{

namespace render
{

static constexpr int swapchainImageCount = 2;
static constexpr size_t defaultBufferSize = 100000000;

Renderer::Renderer(Context& context) :
	context{context},
	device{context.getDevice()},
	graphicsQueue{context.getGraphicQueue(0)},
    commandPool{
        device, 
        graphicsQueue, 
        context.getGraphicsQueueFamilyIndex(), 
        vk::CommandPoolCreateFlagBits::eTransient}
{
    createDescriptorPool();
    createHostBuffer(defaultBufferSize); //arbitrary for now. 100MB
    hostBuffer->map();
    createDeviceBuffer(defaultBufferSize);
    ubos.resize(2);
}

Renderer::~Renderer()
{}

void Renderer::prepareRenderFrames(Window& window)
{
	swapchain = std::make_unique<Swapchain>(context, window, swapchainImageCount); 
    auto& swapchainImages = swapchain->getImages();
	for (auto& imageHandle : swapchainImages) 
	{
		auto image = std::make_unique<Image>(
				device, 
				imageHandle, 
				swapchain->getExtent3D(), 
				swapchain->getFormat(), 
				swapchain->getUsageFlags());
		auto swapAttachment = std::make_unique<Attachment>(device, std::move(image));
		frames.emplace_back(RenderFrame(
				context, 
				std::move(swapAttachment),
				swapchain->getExtent2D().width, 
				swapchain->getExtent2D().height));
	}
}

RenderPass& Renderer::createRenderPass(std::string name)
{
	auto renderPass{RenderPass(device, name)};
	renderPasses.emplace(name, std::move(renderPass));
    std::cout << "made a renderpass" << std::endl;
	renderPassCount++;
	return renderPasses.at(name);
}

const std::string Renderer::createDescriptorSetLayout(const std::string name, const std::vector<vk::DescriptorSetLayoutBinding> bindings)
{
	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.setPBindings(bindings.data());
	createInfo.setBindingCount(bindings.size());
	auto layout = device.createDescriptorSetLayout(createInfo);
	descriptorSetLayouts.emplace(name, std::move(layout)); 
    return name;
}

void Renderer::createFrameDescriptorSets(const std::vector<std::string>setLayoutNames)
{
    std::vector<vk::DescriptorSetLayout> layouts;
    for (auto& name : setLayoutNames) 
    {
        layouts.push_back(*descriptorSetLayouts.at(name));
    }
    for (auto& frame : frames) 
    {
        frame.createDescriptorSets(layouts);
    }
}

void Renderer::createOwnDescriptorSets(const std::vector<std::string>setLayoutNames)
{
    std::vector<vk::DescriptorSetLayout> layouts;
    for (auto& name : setLayoutNames) 
    {
        layouts.push_back(*descriptorSetLayouts.at(name));
    }
	vk::DescriptorSetAllocateInfo ai;
	ai.setPSetLayouts(layouts.data());
	ai.setDescriptorSetCount(layouts.size());
	ai.setDescriptorPool(*descriptorPool);
	descriptorSets = device.allocateDescriptorSetsUnique(ai);
}

void Renderer::addFrameUniformBuffer(size_t size, uint32_t binding)
{
    for (auto& frame : frames) 
    {
        auto block = hostBuffer->requestBlock(size);
        vk::DescriptorBufferInfo bi;
        bi.setRange(block->size);
        bi.setOffset(block->offset);
        bi.setBuffer(hostBuffer->getHandle());
        vk::WriteDescriptorSet bw;
        bw.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        bw.setDstArrayElement(0); //may want to parameterize this
        bw.setDescriptorCount(1);
        bw.setDstSet(*frame.getDescriptorSets().at(0));
        bw.setDstBinding(binding);
        bw.setPBufferInfo(&bi);
        device.updateDescriptorSets(bw, nullptr);
        frame.addUniformBufferBlock(block);
    }
}

void Renderer::updateFrameSamplers(const vk::ImageView* view, const vk::Sampler* sampler, uint32_t binding)
{
    for (auto& frame : frames) 
    {
        vk::DescriptorImageInfo ii;
        if (view)
            ii.setImageView(*view);
        if (sampler)
            ii.setSampler(*sampler);
        ii.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        vk::WriteDescriptorSet iw;
        if (view && sampler)
            iw.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        else if (view)
            iw.setDescriptorType(vk::DescriptorType::eSampledImage);
        else if (sampler)
            iw.setDescriptorType(vk::DescriptorType::eSampler);
        else
            throw std::runtime_error("No option to update frame samplers");
        iw.setDstArrayElement(0);
        iw.setDescriptorCount(1);
        iw.setDstSet(*frame.getDescriptorSets().at(0));
        iw.setDstBinding(binding);
        iw.setPImageInfo(&ii);
        device.updateDescriptorSets(iw, nullptr);
    }
}

void Renderer::updateFrameSamplers(const std::vector<const Image*>& images, uint32_t binding)
{
    for (auto& frame : frames) 
    {
        uint32_t count = images.size();
        assert(count && "Number of images must be greater than 0");
        std::vector<vk::DescriptorImageInfo> imageInfos{count};
        for (uint32_t i = 0; i < count; i++) 
        {
            imageInfos[i].setImageView(images[i]->getView());
            imageInfos[i].setSampler(images[i]->getSampler());
            imageInfos[i].setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        }
        vk::WriteDescriptorSet iw;
        iw.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        iw.setDstArrayElement(0);
        iw.setDstSet(*frame.getDescriptorSets().at(0)); //assuming a single set
        iw.setDstBinding(binding);
        iw.setPImageInfo(imageInfos.data());
        iw.setDescriptorCount(imageInfos.size());
        device.updateDescriptorSets(iw, nullptr);
        std::cout << "Updated descrtiptor sets" << std::endl;
    }
}

void Renderer::updateFrameSamplers(const std::vector<std::string>& attachmentNames, uint32_t binding)
{
    for (auto& frame : frames) 
    {
        uint32_t count = attachmentNames.size();
        assert(count && "Number of images must be greater than 0");
        std::vector<vk::DescriptorImageInfo> imageInfos(count);
        SWD_DEBUG_MSG("Attachment count: " << count);
        for (uint32_t i = 0; i < count; i++) 
        {
            // TODO: we should check flags here
            auto& attachment = attachments.at(attachmentNames[i]);
            imageInfos[i].setImageView(attachment->getImage(0).getView());
            imageInfos[i].setSampler(attachment->getImage(0).getSampler());
            imageInfos[i].setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            SWD_DEBUG_MSG("ImageInfos[" << i << "] image: " << &attachment->getImage(0) );
        }
        vk::WriteDescriptorSet iw;
        iw.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        iw.setDstArrayElement(0);
        iw.setDstSet(*frame.getDescriptorSets().at(0)); //assuming a single set
        iw.setDstBinding(binding);
        iw.setPImageInfo(imageInfos.data());
        iw.setDescriptorCount(imageInfos.size());
        device.updateDescriptorSets(iw, nullptr);
        std::cout << "Updated descrtiptor sets" << std::endl;
    }
}

void Renderer::prepareAsSwapchainPass(RenderPass& rpSwap)
{
    vk::ClearColorValue cv;
    cv.setFloat32({.0,.0,.0,0.});
	rpSwap.createColorAttachment(
			swapchain->getFormat(), 
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR,
            cv,
            vk::AttachmentLoadOp::eClear);
	rpSwap.createSubpass();
	rpSwap.createBasicSubpassDependency();
	rpSwap.create();
}

void Renderer::prepareAsOffscreenPass(
        RenderPass& offScreenPass, vk::AttachmentLoadOp loadOp)
{
    vk::ClearColorValue cv;
    cv.setFloat32({.0,.0,.0,0.});
	offScreenPass.createColorAttachment(
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eShaderReadOnlyOptimal,
            cv,
            loadOp);
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

	offScreenPass.addSubpassDependency(d0);
	offScreenPass.addSubpassDependency(d1);
	offScreenPass.createSubpass();
	offScreenPass.create();
}

Attachment& Renderer::createAttachment(
        const std::string name, const vk::Extent2D extent,
        const vk::ImageUsageFlags usageFlags)
{
    SWD_DEBUG_MSG("Context " << &context);
    SWD_DEBUG_MSG("Context Device " << context.getDevice());
    SWD_DEBUG_MSG("Device " << device);
    auto attachment = std::make_unique<Attachment>(device, extent, usageFlags);
    attachments.emplace(name, std::move(attachment));
    return *attachments.at(name);
}

bool Renderer::createGraphicsPipeline(
		const std::string name, 
        const std::string pipelineLayout,
		const std::string vertShader,
		const std::string fragShader,
		const std::string renderPass,
        const vk::Rect2D renderArea,
		const geo::VertexInfo* vertInfo,
        const vk::PolygonMode polygonMode)
{
    if (graphicsPipelines.find(name) == graphicsPipelines.end())
    {
        std::vector<const Shader*> shaderPointers = 
            {&vertexShaders.at(vertShader), &fragShaderAt(fragShader)};

        const vk::PipelineLayout& layout = *pipelineLayouts.at(pipelineLayout);

        vk::PipelineVertexInputStateCreateInfo vertexState;
        if (vertInfo)
        {
            vertexState.setPVertexBindingDescriptions(vertInfo->getPBindingDescription());
            vertexState.setPVertexAttributeDescriptions(vertInfo->getPAttributeDescriptions());
            vertexState.setVertexBindingDescriptionCount(1);
            vertexState.setVertexAttributeDescriptionCount(vertInfo->getAttributeCount());
        }
        else 
        {
            vertexState.setPVertexBindingDescriptions(nullptr);
            vertexState.setPVertexAttributeDescriptions(nullptr);
            vertexState.setVertexBindingDescriptionCount(0);
            vertexState.setVertexAttributeDescriptionCount(0);
        }

        graphicsPipelines.emplace(name, GraphicsPipeline(
                    name,
                    device,
                    layout,
                    renderPasses.at(renderPass),
                    0,
                    renderArea,
                    shaderPointers,
                    vertexState, 
                    polygonMode));

        return true;
    }
    else
    {
        std::cerr << "Graphics Pipeline with that name already exists." << '\n';
        return false;
    }
}

bool Renderer::recreateGraphicsPipeline(const std::string name)
{
    if (graphicsPipelines.find(name) != graphicsPipelines.end())
    {
        device.waitIdle();
        auto& gp = graphicsPipelines.at(name);
//        for (const auto& frame : frames) 
//        {
//            int id = 0;
//            for (const auto& cmdbuffer : frame.commandBuffers) 
//            {
//                if (cmdbuffer->isRecorded() && cmdbuffer->boundPipeline == &gp.getHandle())
//                {
//                    cmdbuffer->reset();
//                }
//                id++;
//            }
//        }
        std::cout << "Renderer::recreateGraphicsPipeline: calling recreate..." << '\n';
        gp.recreate();
        return true;
    }
    else
    {
        std::cerr << "Renderer::recreateGraphicsPipeline: Graphics pipeline by that name does not exist." << '\n';
        return false;
    }
}

bool Renderer::loadVertShader(
        const std::string path, const std::string name)
{
    if (vertexShaders.find(name) == vertexShaders.end())
    {
        vertexShaders.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(device, path));
        return true;
    }
    return false;
}

bool Renderer::loadVertShader(
        std::vector<uint32_t>&& code, const std::string name)
{
    if (vertexShaders.find(name) == vertexShaders.end())
    {
        vertexShaders.emplace(
            std::piecewise_construct, 
            std::forward_as_tuple(name), 
            std::forward_as_tuple(device, std::move(code)));
        return true;
    }
    return false;
}

bool Renderer::loadFragShader(
		const std::string path, const std::string name)
{
    if (fragmentShaders.find(name) == fragmentShaders.end())
    {
        fragmentShaders.emplace(
                std::piecewise_construct, 
                std::forward_as_tuple(name), 
                std::forward_as_tuple(device, path));
        return true;
    }
    return false;
}

bool Renderer::loadFragShader(
        std::vector<uint32_t>&& code, const std::string name)
{
    if (fragmentShaders.find(name) == fragmentShaders.end())
    {
        fragmentShaders.emplace(
            std::piecewise_construct, 
            std::forward_as_tuple(name), 
            std::forward_as_tuple(device, std::move(code)));
        return true;
    }
    else
    {
        auto& shader = fragmentShaders.at(name);
        shader.reload(std::move(code));
        return true;
    }
}

void Renderer::recordRenderCommands(uint32_t id, std::vector<uint32_t> fbIds)
{
	for (auto& frame : frames) 
	{
		auto& commandBuffer = frame.requestRenderBuffer(id);	
		commandBuffer.begin();

        for (const auto fbId : fbIds)
        {
            auto& renderLayer = frame.getRenderLayer(fbId);
            auto& renderPass = renderLayer.getRenderPass();
            auto& pipeline = renderLayer.getPipeline();
            auto& framebuffer = renderLayer.getFramebuffer();

            auto drawParms = renderLayer.getDrawParms();

            vk::RenderPassBeginInfo bi;
            bi.setFramebuffer(framebuffer);
            bi.setRenderArea(pipeline.getRenderArea());
            bi.setRenderPass(renderPass.getHandle());
            bi.setPClearValues(renderPass.getClearValue());
            bi.setClearValueCount(1);

            commandBuffer.beginRenderPass(bi);
            commandBuffer.bindGraphicsPipeline(pipeline.getHandle());
            commandBuffer.bindDescriptorSets(
                    pipeline.getLayout(),
                    vk::uniqueToRaw(frame.getDescriptorSets()),
                    {});
            auto vertexBuffer = drawParms.getVertexBuffer();
            if (vertexBuffer)
                commandBuffer.bindVertexBuffer(0, drawParms.getVertexBuffer(), drawParms.getOffset());
            commandBuffer.drawVerts(drawParms.getVertexCount(), 0); //default vertexCount is 3
            commandBuffer.endRenderPass();

        }
		commandBuffer.end();
	}

}

void Renderer::createRenderLayer(
        const std::string attachmentName,
        const std::string renderPassName,
        const std::string pipeline,
        const DrawParms drawParms)
{
    auto& rpass = renderPasses.at(renderPassName);
    auto& pipe = graphicsPipelines.at(pipeline);
    for (auto& frame : frames) 
    {
        if (attachmentName.compare("swap") == 0)
            frame.addRenderLayer(rpass, pipe, device, drawParms);
        else 
        {
            auto& attachment = *attachments.at(attachmentName);
            frame.addRenderLayer(RenderLayer(device, attachment, rpass, pipe, drawParms));
        }
    }
}

void Renderer::clearRenderLayers()
{
    for (auto& frame : frames) 
    {
        frame.clearRenderPassInstances();
    }
}

void Renderer::render(uint32_t cmdId, int count, const std::array<int, 5>& ubosToUpdate)
{
	auto& renderBuffer = beginFrame(cmdId);
	assert(renderBuffer.isRecorded() && "Render buffer is not recorded");

    waitOnCommandBuffer(renderBuffer, device);

    for (int i = 0; i < count; i++) 
    {
	    updateFrameDescriptorBuffer(activeFrameIndex, ubosToUpdate[i]);
    }

	//renderBuffer.waitForFence();
	auto submissionCompleteSemaphore = renderBuffer.submit(
			imageAcquiredSemaphore, 
			vk::PipelineStageFlagBits::eColorAttachmentOutput);

	vk::PresentInfoKHR pi;
	pi.setPSwapchains(&swapchain->getHandle());
	pi.setPImageIndices(&activeFrameIndex);
	pi.setSwapchainCount(1);
	pi.setPWaitSemaphores(&submissionCompleteSemaphore);
	pi.setWaitSemaphoreCount(1);

	graphicsQueue.presentKHR(pi);
}

void Renderer::bindUboData(void* dataPointer, uint32_t size, uint32_t index)
{
    ubos.at(index).data = dataPointer;
    ubos.at(index).size = size;
}

CommandBuffer& Renderer::beginFrame(uint32_t cmdId)
{
	//to do: look into storing the imageAcquiredSemaphore in the command buffer itself
	auto& prevFrame = frames.at(activeFrameIndex);
	imageAcquiredSemaphore = prevFrame.requestSemaphore();
	activeFrameIndex = swapchain->acquireNextImage(imageAcquiredSemaphore, nullptr);
	return frames.at(activeFrameIndex).getRenderBuffer(cmdId);
}

void Renderer::createDefaultDescriptorSetLayout(const std::string name)
{
	vk::DescriptorSetLayoutBinding uboBinding;
	uboBinding.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
	uboBinding.setBinding(0);
	uboBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
	uboBinding.setDescriptorCount(1);
	uboBinding.setPImmutableSamplers(nullptr);

	vk::DescriptorSetLayoutBinding samplerBinding;
	samplerBinding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	samplerBinding.setBinding(1);
	samplerBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
	samplerBinding.setDescriptorCount(1);
	samplerBinding.setPImmutableSamplers(nullptr);

	vk::DescriptorSetLayoutBinding fragmentInput;
	fragmentInput.setDescriptorType(vk::DescriptorType::eUniformBuffer);
	fragmentInput.setBinding(2);
	fragmentInput.setStageFlags(vk::ShaderStageFlagBits::eFragment);
	fragmentInput.setDescriptorCount(1);
	fragmentInput.setPImmutableSamplers(nullptr);

	std::vector<vk::DescriptorSetLayoutBinding> bindings{
		uboBinding, 
		samplerBinding,
		fragmentInput};

	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.setPBindings(bindings.data());
	createInfo.setBindingCount(bindings.size());
	auto layout = device.createDescriptorSetLayoutUnique(createInfo);

	descriptorSetLayouts.insert({name, std::move(layout)}); 
	//create a default descriptor set layout presuming one ubo 
	//and one texture sampler
}

const std::string Renderer::createPipelineLayout(const std::string name, const std::vector<std::string> setLayoutNames)
{
	std::vector<vk::DescriptorSetLayout> layouts;
	for (auto layoutName : setLayoutNames) 
	{
		layouts.push_back(*descriptorSetLayouts.at(layoutName));
	}

	vk::PipelineLayoutCreateInfo ci;
	ci.setPSetLayouts(layouts.data());
	ci.setSetLayoutCount(layouts.size());
	ci.setPPushConstantRanges(nullptr);
	ci.setPushConstantRangeCount(0);

	pipelineLayouts.emplace(name, device.createPipelineLayout(ci));
    return name;
}

void Renderer::updateFrameDescriptorBuffer(uint32_t frameIndex, uint32_t uboIndex)
{
	auto block = frames[frameIndex].getUniformBufferBlock(uboIndex);
    assert(block->isMapped && "Block not mapped!");
	auto pHostMemory = block->pHostMemory;
	memcpy(pHostMemory, ubos.at(uboIndex).data, ubos.at(uboIndex).size);
}

void Renderer::createDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 3> sizes;
    sizes[0].setType(vk::DescriptorType::eUniformBuffer);
    sizes[0].setDescriptorCount(1);
    sizes[1].setType(vk::DescriptorType::eSampler);
    sizes[1].setDescriptorCount(1);
    sizes[2].setType(vk::DescriptorType::eSampledImage);
    sizes[2].setDescriptorCount(1000); //arbitary big number
	vk::DescriptorPoolCreateInfo ci;

	ci.setMaxSets(1); //currently allow just one descriptor set
	ci.setPPoolSizes(sizes.data());
	ci.setPoolSizeCount(sizes.size());

	descriptorPool = device.createDescriptorPoolUnique(ci);
}

void Renderer::createHostBuffer(uint32_t size)
{
	hostBuffer = std::make_unique<Buffer>(
			context.getBufferResources(), 
			size, 
			vk::BufferUsageFlagBits::eUniformBuffer |
            vk::BufferUsageFlagBits::eTransferDst |
            vk::BufferUsageFlagBits::eTransferSrc | 
            vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | 
			vk::MemoryPropertyFlagBits::eHostCoherent);
}

void Renderer::createDeviceBuffer(uint32_t size)
{
	deviceBuffer = std::make_unique<Buffer>(
			context.getBufferResources(), 
			size, 
            vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
}

void Renderer::popBufferBlock()
{
    hostBuffer->popBackBlock();
}

void Renderer::listAttachments() const
{
    std::cout << "Attachments: " << std::endl;
    for (const auto& item : attachments) 
    {
        std::cout << item.first << std::endl;
    }
}

void Renderer::listVertShaders() const
{
    std::cout << "Vertex shaders: " << std::endl;
    for (const auto& item : vertexShaders) 
    {
        std::cout << item.first << std::endl;    
    }
}

void Renderer::listFragShaders() const
{
    std::cout << "Fragment Shaders: " << std::endl;
    for (const auto& item : fragmentShaders) 
    {
        std::cout << item.first << std::endl;    
    }
}

void Renderer::listPipelineLayouts() const
{
    std::cout << "Pipeline layouts: " << std::endl;
    for (const auto& item : pipelineLayouts)
    {
        std::cout << item.first << std::endl;
    }
}

void Renderer::listRenderPasses() const
{
    std::cout << "RenderPasses: " << std::endl;
    for (const auto& item : renderPasses)
    {
        std::cout << item.first << std::endl;
    }
}

FragShader& Renderer::fragShaderAt(const std::string name)
{
    return fragmentShaders.at(name);
}

VertShader& Renderer::vertShaderAt(const std::string name)
{
    return vertexShaders.at(name);
}


BufferBlock* Renderer::copySwapToHost()
{
    auto extent = swapchain->getExtent2D();
    auto block = hostBuffer->requestBlock(
            extent.width * extent.height * 4);

    auto& commandBuffer = commandPool.requestCommandBuffer();

    vk::ImageSubresourceRange isr;
    isr.setAspectMask(vk::ImageAspectFlagBits::eColor);
    isr.setLayerCount(1);
    isr.setLevelCount(1);
    isr.setBaseMipLevel(0);
    isr.setBaseArrayLayer(0);

    auto& swapAttachment = frames.at(activeFrameIndex).getSwapAttachment();
    auto& image = swapAttachment.getImage(0).getImage();
    vk::ImageMemoryBarrier imb;
    imb.setImage(image);
    imb.setOldLayout(vk::ImageLayout::ePresentSrcKHR);
    imb.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
    imb.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
    imb.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
    imb.setSubresourceRange(isr);

    commandBuffer.begin();
    commandBuffer.insertImageMemoryBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            imb);

    vk::BufferImageCopy copyRegion;
    copyRegion.setImageExtent({extent.width, extent.height, 1});
    copyRegion.setImageOffset({0, 0, 0});
    copyRegion.setBufferOffset(block->offset);
    copyRegion.setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1});
    copyRegion.setBufferRowLength(0);
    copyRegion.setBufferImageHeight(0);

    commandBuffer.copyImageToBuffer(image, hostBuffer->getHandle(), copyRegion);

    commandBuffer.end(); //renderpass should take care of converting the swap image layout back for us

    commandBuffer.submit();

    commandBuffer.waitForFence();

    commandPool.resetPool();
    return block;
}

BufferBlock* Renderer::copyAttachmentToHost(
        const std::string name, const vk::Rect2D region)
{
    auto block = hostBuffer->requestBlock(
            region.extent.width * region.extent.height * 4);

    auto& commandBuffer = commandPool.requestCommandBuffer();

    vk::ImageSubresourceRange isr;
    isr.setAspectMask(vk::ImageAspectFlagBits::eColor);
    isr.setLayerCount(1);
    isr.setLevelCount(1);
    isr.setBaseMipLevel(0);
    isr.setBaseArrayLayer(0);

    auto& attachment = attachments.at(name);
    auto& image = attachment->getImage(0).getImage();
    vk::ImageMemoryBarrier imb;
    imb.setImage(image);
    imb.setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    imb.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
    imb.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
    imb.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
    imb.setSubresourceRange(isr);

    commandBuffer.begin();
    commandBuffer.insertImageMemoryBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            imb);

    vk::BufferImageCopy copyRegion;
    copyRegion.setImageExtent({region.extent.width, region.extent.height, 1});
    copyRegion.setImageOffset({region.offset.x, region.offset.y, 0});
    copyRegion.setBufferOffset(block->offset);
    copyRegion.setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1});
    copyRegion.setBufferRowLength(0);
    copyRegion.setBufferImageHeight(0);

    commandBuffer.copyImageToBuffer(image, hostBuffer->getHandle(), copyRegion);

    commandBuffer.end(); //renderpass should take care of converting the swap image layout back for us

    commandBuffer.submit();

    commandBuffer.waitForFence();

    commandPool.resetPool();
    return block;
}

BufferBlock* Renderer::requestHostBufferBlock(size_t size)
{
    return hostBuffer->requestBlock(size);
}

BufferBlock* Renderer::requestDeviceBufferBlock(size_t size)
{
    return deviceBuffer->requestBlock(size);
}


void Renderer::copyHostToAttachment(void* source, int size, std::string attachmentName, const vk::Rect2D region)
{
    assert(size == region.extent.width * region.extent.height * 4 && "size does not match region");

    auto block = hostBuffer->requestBlock(size);

    auto& commandBuffer = commandPool.requestCommandBuffer();

    vk::ImageSubresourceRange isr;
    isr.setAspectMask(vk::ImageAspectFlagBits::eColor);
    isr.setLayerCount(1);
    isr.setLevelCount(1);
    isr.setBaseMipLevel(0);
    isr.setBaseArrayLayer(0);

    auto& attachment = attachments.at(attachmentName);
    auto& image = attachment->getImage(0).getImage();

    vk::ImageMemoryBarrier imb;
    imb.setImage(image);
    imb.setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    imb.setNewLayout(vk::ImageLayout::eTransferDstOptimal);
    imb.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
    imb.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
    imb.setSubresourceRange(isr);

    commandBuffer.begin();
    commandBuffer.insertImageMemoryBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            imb);

    vk::BufferImageCopy copyRegion;
    copyRegion.setImageExtent({region.extent.width, region.extent.height, 1});
    copyRegion.setImageOffset({region.offset.x, region.offset.y, 0});
    copyRegion.setBufferOffset(block->offset);
    copyRegion.setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1});
    copyRegion.setBufferRowLength(0);
    copyRegion.setBufferImageHeight(0);

    //TODO: finish this
}

Attachment* Renderer::getAttachmentPtr(std::string name) const
{
    return attachments.at(name).get();
}

vk::Extent2D Renderer::getSwapExtent()
{
    return swapchain->getExtent2D();
}


} // namespace render

} // namespace sword


