#include <render/context.hpp>
#include <render/resource.hpp>
#include <render/swapchain.hpp>
#include <render/renderframe.hpp>
#include <render/renderlayer.hpp>
#include <render/attachment.hpp>
#include <render/renderer.hpp>

namespace sword
{

namespace render
{

Renderer::Renderer(Context& context) :
	context{context},
	device{context.device},
	graphicsQueue{context.queue},
    commandPool{
        device, 
        graphicsQueue, 
        context.getGraphicsQueueFamilyIndex(), 
        vk::CommandPoolCreateFlagBits::eTransient}
{
    createDescriptorPool();
    createDescriptorBuffer(100000000); //arbitrary for now. 100MB
    descriptorBuffer->map();
    ubos.resize(1);
}

Renderer::~Renderer()
{
	for (auto item: descriptorSetLayouts)
	{
		device.destroyDescriptorSetLayout(item.second);
	}

	for (auto item : pipelineLayouts) 
	{
		device.destroyPipelineLayout(item.second);
	}
	
	device.waitIdle();
    device.destroyDescriptorPool(descriptorPool);
	frames.clear();

	descriptorSetLayouts.clear();
	pipelineLayouts.clear();
}

void Renderer::prepareRenderFrames(Window& window)
{
	swapchain = std::make_unique<Swapchain>(context, window, 3); 
	//we hardcode 3 swap images
	for (auto& imageHandle : swapchain->getImages()) 
	{
		auto image = std::make_unique<Image>(
				device, 
				imageHandle, 
				swapchain->getExtent3D(), 
				swapchain->getFormat(), 
				swapchain->getUsageFlags());
		auto swapAttachment{
			std::make_unique<Attachment>(device, std::move(image))};
		auto renderFrame{RenderFrame(
				context, 
				std::move(swapAttachment),
				swapchain->getExtent2D().width, 
				swapchain->getExtent2D().height)};
		frames.emplace_back(std::move(renderFrame));
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
        layouts.push_back(descriptorSetLayouts.at(name));
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
        layouts.push_back(descriptorSetLayouts.at(name));
    }
	vk::DescriptorSetAllocateInfo ai;
	ai.setPSetLayouts(layouts.data());
	ai.setDescriptorSetCount(layouts.size());
	ai.setDescriptorPool(descriptorPool);
	descriptorSets = device.allocateDescriptorSets(ai);
}

void Renderer::initFrameUBOs(size_t size, uint32_t binding)
{
    for (auto& frame : frames) 
    {
        auto block = descriptorBuffer->requestBlock(size);
        vk::DescriptorBufferInfo bi;
        bi.setRange(block->size);
        bi.setOffset(block->offset);
        bi.setBuffer(descriptorBuffer->getHandle());
        vk::WriteDescriptorSet bw;
        bw.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        bw.setDstArrayElement(0); //may want to parameterize this
        bw.setDescriptorCount(1);
        bw.setDstSet(frame.getDescriptorSets().at(0));
        bw.setDstBinding(binding);
        bw.setPBufferInfo(&bi);
        device.updateDescriptorSets(bw, nullptr);
        frame.bufferBlock = block;
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
        iw.setDstSet(frame.getDescriptorSets().at(0));
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
        iw.setDstSet(frame.getDescriptorSets().at(0)); //assuming a single set
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
    auto attachment = std::make_unique<Attachment>(device, extent, usageFlags);
    attachments.emplace(name, std::move(attachment));
    return *attachments.at(name);
}

GraphicsPipeline& Renderer::createGraphicsPipeline(
		const std::string name, 
        const std::string pipelineLayout,
		const std::string vertShader,
		const std::string fragShader,
		const std::string renderPass,
        const vk::Rect2D renderArea,
		const bool geometric)
{
    assert(graphicsPipelines.find(name) == graphicsPipelines.end() && "Duplicate name for graphics pipeline");

	std::vector<const Shader*> shaderPointers = 
        {&vertexShaders.at(vertShader), &fragShaderAt(fragShader)};

	vk::PipelineLayout layout = pipelineLayouts.at(pipelineLayout);

    vk::PipelineVertexInputStateCreateInfo vertexState;
	if (geometric)
	{
		assert(descriptionIsBound);
//		auto attributeDescriptions = pDescription->getAttributeDescriptions();
//		auto vertexBindingDescriptions = pDescription->getBindingDescriptions();
//		vertexState = GraphicsPipeline::createVertexInputState(
//				attributeDescriptions, vertexBindingDescriptions);
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
				vertexState));

    return graphicsPipelines.at(name);
}

VertShader& Renderer::loadVertShader(
        const std::string path, const std::string name)
{
    assert(vertexShaders.find(name) == vertexShaders.end() && "Shader name is not unique");
    vertexShaders.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(device, path));
    return vertexShaders.at(name);
}

FragShader& Renderer::loadFragShader(
		const std::string path, const std::string name)
{
    assert(fragmentShaders.find(name) == fragmentShaders.end() && "Shader name is not unique");
    fragmentShaders.emplace(
            std::piecewise_construct, 
            std::forward_as_tuple(name), 
            std::forward_as_tuple(device, path));
    return fragmentShaders.at(name);
}

void Renderer::recordRenderCommands(uint32_t id, std::vector<uint32_t> fbIds)
{
	for (auto& frame : frames) 
	{
		auto& commandBuffer = frame.requestRenderBuffer(id);	
		commandBuffer.begin();

        for (auto fbId : fbIds)
        {
            auto& renderPassInstance = frame.getRenderLayer(fbId);
            auto& renderPass = renderPassInstance.getRenderPass();
            auto& pipeline = renderPassInstance.getPipeline();
            auto& framebuffer = renderPassInstance.getFramebuffer();

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
                    frame.getDescriptorSets(),
                    {});
            commandBuffer.drawVerts(3, 0);
            commandBuffer.endRenderPass();

        }
		commandBuffer.end();
	}
}

void Renderer::addRenderPassInstance(
        std::string attachmentName,
        std::string renderPassName,
        std::string pipeline)
{
    auto& rpass = renderPasses.at(renderPassName);
    auto& pipe = graphicsPipelines.at(pipeline);
    for (auto& frame : frames) 
    {
        if (attachmentName.compare("swap") == 0)
            frame.addRenderPassInstance(rpass, pipe);
        else 
        {
            auto& attachment = *attachments.at(attachmentName);
            frame.addRenderPassInstance(attachment, rpass, pipe);
        }
    }
}

void Renderer::clearRenderPassInstances()
{
    for (auto& frame : frames) 
    {
        frame.clearRenderPassInstances();
    }
}

void Renderer::render(uint32_t cmdId, bool updateUbo)
{
	auto& renderBuffer = beginFrame(cmdId);
	assert(renderBuffer.isRecorded() && "Render buffer is not recorded");

    if (updateUbo)
	    updateFrameDescriptorBuffer(activeFrameIndex, 0);

	renderBuffer.waitForFence();
	auto submissionCompleteSemaphore = renderBuffer.submit(
			imageAcquiredSemaphore, 
			vk::PipelineStageFlagBits::eColorAttachmentOutput);

	vk::PresentInfoKHR pi;
	pi.setPSwapchains(&swapchain->getHandle());
	pi.setPImageIndices(&activeFrameIndex);
	pi.setSwapchainCount(1);
	pi.setPWaitSemaphores(&submissionCompleteSemaphore);
	pi.setWaitSemaphoreCount(1);

	context.queue.presentKHR(pi);
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
	auto layout = device.createDescriptorSetLayout(createInfo);

	descriptorSetLayouts.insert({name, std::move(layout)}); 
	//create a default descriptor set layout presuming one ubo 
	//and one texture sampler
}

const std::string Renderer::createPipelineLayout(const std::string name, const std::vector<std::string> setLayoutNames)
{
	std::vector<vk::DescriptorSetLayout> layouts;
	for (auto layoutName : setLayoutNames) 
	{
		layouts.push_back(descriptorSetLayouts.at(layoutName));
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
	auto block = descriptorBuffer->bufferBlocks[frameIndex].get();
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

	descriptorPool = device.createDescriptorPool(ci);
}

void Renderer::createDescriptorBuffer(uint32_t size)
{
	descriptorBuffer = std::make_unique<Buffer>(
			device, 
            context.physicalDeviceProperties,
			context.physicalDeviceMemoryProperties, 
			size, 
			vk::BufferUsageFlagBits::eUniformBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eHostVisible | 
			vk::MemoryPropertyFlagBits::eHostCoherent);
}

void Renderer::popBufferBlock()
{
    descriptorBuffer->popBackBlock();
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


BufferBlock* Renderer::copySwapToHost(const vk::Rect2D region)
{
    auto block = descriptorBuffer->requestBlock(
            region.extent.width * region.extent.height * 4);

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
    copyRegion.setImageExtent({region.extent.width, region.extent.height, 1});
    copyRegion.setImageOffset({region.offset.x, region.offset.y, 0});
    copyRegion.setBufferOffset(block->offset);
    copyRegion.setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1});
    copyRegion.setBufferRowLength(0);
    copyRegion.setBufferImageHeight(0);

    commandBuffer.copyImageToBuffer(image, descriptorBuffer->getHandle(), copyRegion);

    commandBuffer.end(); //renderpass should take care of converting the swap image layout back for us

    commandBuffer.submit();

    commandBuffer.waitForFence();

    commandPool.resetPool();
    return block;
}

BufferBlock* Renderer::copyAttachmentToHost(
        const std::string name, const vk::Rect2D region)
{
    auto block = descriptorBuffer->requestBlock(
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

    commandBuffer.copyImageToBuffer(image, descriptorBuffer->getHandle(), copyRegion);

    commandBuffer.end(); //renderpass should take care of converting the swap image layout back for us

    commandBuffer.submit();

    commandBuffer.waitForFence();

    commandPool.resetPool();
    return block;
}



}; // namespace render

}; // namespace sword


