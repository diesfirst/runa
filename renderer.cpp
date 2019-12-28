//storage space for now, but may be a class for 3D to 2D conversions
#include <vector>
#include "renderer.hpp"
#include "io.hpp"
#include <iostream>
#include <fstream>


Shader::Shader(const vk::Device& device, std::string filepath) :
	device{device}
{
	loadFile(filepath);
	createModule();
	stageInfo.setPName("main");
	stageInfo.setModule(module);
	std::cout << "Shader constructed" << std::endl;
}

Shader::~Shader()
{
	if (module)
		device.destroyShaderModule(module);
}

//movement is tricky. going to disallow it for now
//
//Shader::Shader(Shader&& other) :
//	device{other.device},
//	shaderCode{std::move(other.shaderCode)},
//	codeSize{other.codeSize},
//	module{std::move(other.module)},
//	stageInfo{std::move(other.stageInfo)},
//	mapEntries{std::move(other.mapEntries)},
//	specInfo{std::move(other.specInfo)},
//	specializationFloats{std::move(other.specializationFloats)}
//{
//	std::cout << "shader move ctor called" << std::endl;
//	other.module = nullptr;
//	specInfo.setPMapEntries(mapEntries.data());
//	specInfo.setPData(specializationFloats.data());
//}

const vk::PipelineShaderStageCreateInfo& Shader::getStageInfo() const
{
	return stageInfo;
}

void Shader::loadFile(std::string filepath)
{
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);
	//ate: start reading at end of file. allows us to get the size of the file
	//binary: read the file as binary file
	assert(file.is_open() && "Failed to load shader file");
	codeSize = (size_t) file.tellg();
	shaderCode.resize(codeSize);

	file.seekg(0);
	file.read(shaderCode.data(), codeSize);
	file.close();
}

void Shader::createModule()
{
	vk::ShaderModuleCreateInfo ci;
	ci.setPCode(reinterpret_cast<const uint32_t*>(shaderCode.data()));
	ci.setCodeSize(codeSize);
	module = device.createShaderModule(ci);
}

void Shader::setWindowResolution(const uint32_t w, const uint32_t h)
{
	mapEntries.resize(2);
	mapEntries[0].setSize(sizeof(float));
	mapEntries[0].setOffset(0 * sizeof(float));
	mapEntries[0].setConstantID(0);
	mapEntries[1].setSize(sizeof(float));
	mapEntries[1].setOffset(1 * sizeof(float));
	mapEntries[1].setConstantID(1);

	specializationFloats.resize(2);
	specializationFloats[0] = w;
	specializationFloats[1] = h;

	specInfo.setPMapEntries(mapEntries.data());
	specInfo.setMapEntryCount(2);
	specInfo.setPData(specializationFloats.data());
	specInfo.setDataSize(sizeof(float) * specializationFloats.size());

	stageInfo.setPSpecializationInfo(&specInfo);
}

VertShader::VertShader(const vk::Device& device, std::string filepath) :
	Shader(device, filepath)
{
	stageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
}

FragShader::FragShader(const vk::Device& device, std::string filepath) :
	Shader(device, filepath)
{
	stageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
}

RenderPass::RenderPass(const vk::Device& device, const vk::Format format, uint32_t id) :
	device{device},
	id{id}
{
	std::vector<vk::AttachmentDescription> attachments;
	std::vector<vk::AttachmentReference> references;
	std::vector<vk::SubpassDescription> subpasses;
	std::vector<vk::SubpassDependency> subpassDependencies;

	vk::AttachmentDescription attachment;
	attachment.setFormat(format);
	attachment.setSamples(vk::SampleCountFlagBits::e1);
	//Sets what to do with data in the attachment
	//before rendering
	attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
	attachment.setInitialLayout(vk::ImageLayout::eUndefined);
	//Sets what we do with the data after rendering
	//We want to show it so we will store it
	attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
	attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	attachments.push_back(attachment);

	vk::AttachmentReference colorRef;
	colorRef.setAttachment(0); //our first attachment is color
	colorRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	references.push_back(colorRef);

	vk::SubpassDescription subpass;
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpass.setColorAttachmentCount(1);
	subpass.setPColorAttachments(references.data());
	subpasses.push_back(subpass);

	vk::SubpassDependency dependency;
	dependency.setSrcSubpass(
			VK_SUBPASS_EXTERNAL);
	dependency.setDstSubpass(
			0);
	dependency.setSrcAccessMask(
			vk::AccessFlagBits::eShaderRead);
	dependency.setDstAccessMask(
			vk::AccessFlagBits::eColorAttachmentWrite);
	dependency.setSrcStageMask(
			vk::PipelineStageFlagBits::eFragmentShader);
	dependency.setDstStageMask(
			vk::PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependencies.push_back(dependency);

	vk::RenderPassCreateInfo createInfo;
	createInfo.setAttachmentCount(attachments.size());
	createInfo.setPAttachments(attachments.data());
	createInfo.setSubpassCount(subpasses.size());
	createInfo.setPSubpasses(subpasses.data());
	createInfo.setDependencyCount(subpassDependencies.size());
	createInfo.setPDependencies(subpassDependencies.data());
  	handle = device.createRenderPass(createInfo);
}

RenderPass::~RenderPass()
{
	if (handle)
		device.destroyRenderPass(handle);
}

RenderPass::RenderPass(RenderPass&& other) :
	device{other.device},
	id{other.id},
	handle{other.handle}
{
	other.handle = nullptr;
}

bool RenderPass::operator<(const RenderPass& rhs)
{
	return handle == rhs.handle;
}


const vk::RenderPass& RenderPass::getHandle() const
{
	return handle;
}

uint32_t RenderPass::getId() const
{
	return id;
}

RenderTarget::RenderTarget(std::unique_ptr<mm::Image> swapimage)
{
	images.push_back(std::move(swapimage));
}

RenderTarget::~RenderTarget()
{
}

RenderFrame::RenderFrame(
		const Context& context, 
		std::unique_ptr<RenderTarget>&& renderTarget,
		uint32_t width, uint32_t height) :
	context(context),
	device(context.device),
	swapchainRenderTarget{std::move(renderTarget)},
	commandPool{CommandPool(device, context.queue, context.getGraphicsQueueFamilyIndex())},
	width{width},
	height{height}
{
	vk::SemaphoreCreateInfo semaInfo;
	vk::FenceCreateInfo fenceInfo;
	semaphore = device.createSemaphore(semaInfo);
	fence = device.createFence(fenceInfo);
	createDescriptorPool();
	createDescriptorBuffer(true);
}

RenderFrame::~RenderFrame()
{
	if (semaphore)
		device.destroySemaphore(semaphore);
	if (fence)	
		device.destroyFence(fence);
	if (descriptorPool)
		device.destroyDescriptorPool(descriptorPool);
	for (auto item : frameBuffers) 
	{
		device.destroyFramebuffer(item.second);	
	}
}

RenderFrame::RenderFrame(RenderFrame&& other) :
	context(other.context),
	device(other.device),
	commandPool(std::move(other.commandPool)),
	fence(other.fence),
	semaphore(other.semaphore),
	descriptorPool(other.descriptorPool),
	descriptorSets{std::move(other.descriptorSets)},
	width{other.width},
	height{other.height},
	renderBuffer{other.renderBuffer},
	frameBuffers{std::move(other.frameBuffers)},
	swapchainRenderTarget{std::move(other.swapchainRenderTarget)},
	descriptorBuffer{std::move(other.descriptorBuffer)}
{
	other.semaphore = nullptr;
	other.fence = nullptr;
	other.descriptorPool = nullptr;
	other.frameBuffers.clear();
}

void RenderFrame::createDescriptorBuffer(bool map)
{
	descriptorBuffer = std::make_unique<mm::Buffer>(
			device, 
			context.physicalDeviceMemoryProperties, 
			200, 
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | 
			vk::MemoryPropertyFlagBits::eHostCoherent);
	if (map)
		auto res = *descriptorBuffer->map(); //do not intend to use this yet
}

mm::Buffer& RenderFrame::getDescriptorBuffer()
{
	return *descriptorBuffer;
}

vk::Framebuffer& RenderFrame::createFramebuffer(
		const RenderPass& renderPass)
{
	std::array<vk::ImageView, 1> attachments;
	attachments[0] = swapchainRenderTarget->images[0]->getView();

	vk::FramebufferCreateInfo ci;
	ci.setWidth(width);
	ci.setHeight(height);
	ci.setAttachmentCount(attachments.size());
	ci.setPAttachments(attachments.data());
	ci.setLayers(1);
	ci.setRenderPass(renderPass.getHandle());
	frameBuffers.emplace(renderPass.getId(), device.createFramebuffer(ci));
	return frameBuffers.at(renderPass.getId());
}

vk::Framebuffer& RenderFrame::requestFrameBuffer(const RenderPass& renderPass)
{
	if (frameBuffers.find(renderPass.getId()) == frameBuffers.end())
	{
		return createFramebuffer(renderPass);
	}
	else
		return frameBuffers.at(renderPass.getId());
}

CommandBuffer& RenderFrame::getRenderBuffer()
{
	if (!renderBuffer)
	{
		renderBuffer = &requestCommandBuffer();
		return *renderBuffer;
	}
	else
		return *renderBuffer;
}

vk::Semaphore RenderFrame::requestSemaphore()
{
	return semaphore;
}

CommandBuffer& RenderFrame::requestCommandBuffer()
{	
	return commandPool.requestCommandBuffer();
}

void RenderFrame::createDescriptorPool()
{
	vk::DescriptorPoolSize uboDynamicSize;
	vk::DescriptorPoolSize imageSamplerSize;
	vk::DescriptorPoolSize uboSize;
	uboDynamicSize.setType(vk::DescriptorType::eUniformBufferDynamic);
	uboDynamicSize.setDescriptorCount(1);
	imageSamplerSize.setType(vk::DescriptorType::eCombinedImageSampler);
	imageSamplerSize.setDescriptorCount(1);
	uboSize.setType(vk::DescriptorType::eUniformBuffer);
	uboSize.setDescriptorCount(1);

	std::array<vk::DescriptorPoolSize, 3> sizes{uboSize, imageSamplerSize, uboDynamicSize};

	vk::DescriptorPoolCreateInfo ci;
	ci.setMaxSets(1); //currently allow just one descriptor set
	ci.setPPoolSizes(sizes.data());
	ci.setPoolSizeCount(sizes.size());

	descriptorPool = device.createDescriptorPool(ci);
}

void RenderFrame::createDescriptorSet(std::vector<vk::DescriptorSetLayout>& layouts)
{
	vk::DescriptorSetAllocateInfo ai;
	ai.setPSetLayouts(layouts.data());
	ai.setDescriptorSetCount(layouts.size());
	ai.setDescriptorPool(descriptorPool);
	descriptorSets = device.allocateDescriptorSets(ai);
	
	vk::DescriptorBufferInfo bi;
	bi.setRange(sizeof(FragmentInput));
	bi.setBuffer((*descriptorBuffer).getHandle());
	bi.setOffset(0);

	vk::WriteDescriptorSet dWrite;
	dWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
	dWrite.setDstArrayElement(0);
	dWrite.setDstSet(descriptorSets.at(0));
	dWrite.setDstBinding(2);
	dWrite.setPImageInfo(nullptr);
	dWrite.setPBufferInfo(&bi);
	dWrite.setDescriptorCount(1);
	dWrite.setPTexelBufferView(nullptr);

	device.updateDescriptorSets(dWrite, nullptr);
}

const std::vector<vk::DescriptorSet>& RenderFrame::getDescriptorSets() const
{
	return descriptorSets;
}

GraphicsPipeline::GraphicsPipeline(
		const vk::Device& device, 
		const vk::PipelineLayout& layout,
		const vk::RenderPass& renderPass,
		const uint32_t subpassIndex,
		const vk::Rect2D renderArea,
		const std::vector<const Shader*>& shaders,
		const vk::PipelineVertexInputStateCreateInfo& vertexInputState) :
	device{device},
	layout{layout}
{
	vk::Viewport viewport = createViewport(renderArea);
	vk::Rect2D scissor = renderArea;

	auto attachmentState0 = createColorBlendAttachmentState();
	std::vector<vk::PipelineColorBlendAttachmentState> attachmentStates{attachmentState0};

	auto shaderStageInfos = extractShaderStageInfos(shaders);
	auto viewportState = createViewportState(viewport, scissor);
	auto multisampleState = createMultisampleState();
	auto rasterizationState = createRasterizationState();
	auto colorBlendState = createColorBlendState(attachmentStates);
	auto depthStencilState = createDepthStencilState(); //not ready yet
	auto inputAssemblySate = createInputAssemblyState();

	vk::GraphicsPipelineCreateInfo ci;
	ci.setLayout(layout);
	ci.setPStages(shaderStageInfos.data());
	ci.setStageCount(shaders.size());
	ci.setSubpass(subpassIndex);
	ci.setRenderPass(renderPass);
	ci.setPDynamicState(nullptr); 
	//we could use this to specify some pipeline elements
	//as dynamic. worth looking into.
	ci.setPViewportState(&viewportState);
	ci.setPMultisampleState(&multisampleState);
	ci.setPTessellationState(nullptr);
	ci.setPRasterizationState(&rasterizationState);
	ci.setPColorBlendState(&colorBlendState);
	ci.setBasePipelineHandle(nullptr);
	ci.setBasePipelineIndex(0);
	ci.setPVertexInputState(&vertexInputState);
	ci.setPDepthStencilState(nullptr); //null for now
	ci.setPInputAssemblyState(&inputAssemblySate);
	handle = device.createGraphicsPipeline({}, ci);
}

GraphicsPipeline::~GraphicsPipeline()
{
	if (handle)
		device.destroyPipeline(handle);
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) :
	device{other.device},
	handle{std::move(other.handle)},
	layout{std::move(other.layout)}
{
	other.handle = nullptr;
}

vk::Pipeline& GraphicsPipeline::getHandle()
{
	return handle;
}

vk::PipelineLayout& GraphicsPipeline::getLayout()
{
	return layout;
}

vk::Viewport GraphicsPipeline::createViewport(const vk::Rect2D& renderArea)
{
	vk::Viewport viewport;
	viewport.x = renderArea.offset.x;
	viewport.y = renderArea.offset.y;
	viewport.width = renderArea.extent.width;
	viewport.height = renderArea.extent.height;
	viewport.minDepth = 0.0;
	viewport.maxDepth = 1.0;
	return viewport;
}

vk::PipelineViewportStateCreateInfo GraphicsPipeline::createViewportState(
		const vk::Viewport& viewport,
		const vk::Rect2D& scissor)
{
	vk::PipelineViewportStateCreateInfo ci;
	ci.setPScissors(&scissor);
	ci.setPViewports(&viewport);
	ci.setScissorCount(1);
	ci.setViewportCount(1);
	return ci;
}

vk::PipelineMultisampleStateCreateInfo GraphicsPipeline::createMultisampleState()
{
	vk::PipelineMultisampleStateCreateInfo ci;
	//turned sample shading on from physicalDevice features
	//but currently will do nothing since its disables here
	ci.setSampleShadingEnable(false);
	ci.setAlphaToOneEnable(false);
	ci.setAlphaToCoverageEnable(false);
	ci.setRasterizationSamples(vk::SampleCountFlagBits::e1);
	ci.setPSampleMask(nullptr);
	ci.setMinSampleShading(1.0);
	return ci;
}

vk::PipelineRasterizationStateCreateInfo GraphicsPipeline::createRasterizationState()
{
	vk::PipelineRasterizationStateCreateInfo ci;
	ci.setDepthBiasEnable(false);
	ci.setDepthClampEnable(false);
	ci.setRasterizerDiscardEnable(false);
	ci.setCullMode(vk::CullModeFlagBits::eBack);
	ci.setFrontFace(vk::FrontFace::eClockwise);
	ci.setLineWidth(1.0);
	ci.setPolygonMode(vk::PolygonMode::eFillRectangleNV);
	ci.setDepthBiasClamp(0.0);
	ci.setDepthBiasSlopeFactor(0.0);
	ci.setDepthBiasConstantFactor(0.0);
	return ci;
}

vk::PipelineColorBlendStateCreateInfo GraphicsPipeline::createColorBlendState(
		std::vector<vk::PipelineColorBlendAttachmentState>& attachmentStates)
{
	vk::PipelineColorBlendStateCreateInfo ci;
	ci.setLogicOpEnable(false);
	ci.setPAttachments(attachmentStates.data());
	ci.setAttachmentCount(attachmentStates.size());
	ci.setLogicOp(vk::LogicOp::eCopy);
	ci.setBlendConstants({0,0,0,0});
	return ci;
}

vk::PipelineColorBlendAttachmentState GraphicsPipeline::createColorBlendAttachmentState()
{
	vk::ColorComponentFlags writeMask = 
		vk::ColorComponentFlagBits::eA |
		vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eR;

	vk::PipelineColorBlendAttachmentState ci;
	ci.setBlendEnable(true);
	ci.setAlphaBlendOp(vk::BlendOp::eAdd);
	ci.setColorBlendOp(vk::BlendOp::eAdd);
	ci.setColorWriteMask(writeMask);
	ci.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
	ci.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
	ci.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
	ci.setSrcColorBlendFactor(vk::BlendFactor::eOne);
	return ci;
}

vk::PipelineVertexInputStateCreateInfo GraphicsPipeline::createVertexInputState(
		const std::vector<vk::VertexInputAttributeDescription>& attributeDescriptions,
		const std::vector<vk::VertexInputBindingDescription>& bindingDescriptions)
{
	vk::PipelineVertexInputStateCreateInfo ci;
	ci.setPVertexAttributeDescriptions(attributeDescriptions.data());
	ci.setVertexAttributeDescriptionCount(attributeDescriptions.size());
	ci.setPVertexBindingDescriptions(bindingDescriptions.data());
	ci.setVertexBindingDescriptionCount(bindingDescriptions.size());
	return ci;
}

vk::PipelineDepthStencilStateCreateInfo GraphicsPipeline::createDepthStencilState()
{
	vk::PipelineDepthStencilStateCreateInfo ci;
	ci.setDepthTestEnable(false); //set depth testing off for now
	ci.setDepthWriteEnable(false);
	ci.setStencilTestEnable(false);
	ci.setDepthBoundsTestEnable(false);
	//this is incomplete
	return ci;
}

vk::PipelineInputAssemblyStateCreateInfo GraphicsPipeline::createInputAssemblyState()
{
	vk::PipelineInputAssemblyStateCreateInfo ci;
	ci.setTopology(vk::PrimitiveTopology::eTriangleList);
	ci.setPrimitiveRestartEnable(false);
	return ci;
}

std::vector<vk::PipelineShaderStageCreateInfo> GraphicsPipeline::extractShaderStageInfos(
		const std::vector<const Shader*>& shaders)
{
	std::vector<vk::PipelineShaderStageCreateInfo> stageInfos;
	stageInfos.resize(shaders.size());
	for (int i = 0; i < shaders.size(); i++) 
	{
		stageInfos[i] = shaders[i]->getStageInfo();
	}
	return stageInfos;
}

Renderer::Renderer(Context& context, XWindow& window) :
	context(context),
	device(context.device),
	graphicsQueue(context.queue)
{
	swapchain = std::make_unique<Swapchain>(context, window, 3); //we hardcode 3 swap images
	for (auto& imageHandle : swapchain->getImages()) 
	{
		auto image = std::make_unique<mm::Image>(
				device, 
				imageHandle, 
				swapchain->getExtent3D(), 
				swapchain->getFormat(), 
				swapchain->getUsageFlags());
		auto renderTarget{std::make_unique<RenderTarget>(std::move(image))};
		auto renderFrame{RenderFrame(context, std::move(renderTarget),
				swapchain->getExtent2D().width, swapchain->getExtent2D().height)};
		frames.emplace_back(std::move(renderFrame));
	}
	//the following code will handle descriptor set creation
	//while we don't need descriptor sets to create a Pipeline
	//we do need to make the Pipeline aware of their layouts 
	//if we do intend to use them
	//----------------------------
	createDefaultDescriptorSetLayout("default");
	//we can now create the desriptorsets for the render frames 
	//additionally we can create our pipeline layouts
	createPipelineLayout("default", {"default"});

	//note: this could have been done as soon as we had the layout
	createDescriptorSets(frames, {"default"});
	//----------------------------
	//
	//we'll create renderpass now. 
	//note: we could have done this at any point
	createRenderPass("default", swapchain->getFormat());
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
	frames.clear();

	descriptorSetLayouts.clear();
	pipelineLayouts.clear();
}

void Renderer::recordRenderCommands(const std::string pipelineName, const std::string renderPassName)
{
	for (auto& frame : frames) 
	{
		auto& commandBuffer = frame.getRenderBuffer();	
		commandBuffer.begin();

		auto& renderPass = renderPasses.at(renderPassName);
		auto& frameBuffer = frame.requestFrameBuffer(renderPass);

		//could add the render area and clear color data to the 
		//RenderFrames, might make sense
		vk::Rect2D renderArea;
		renderArea.setOffset({0,0});
		renderArea.setExtent(swapchain->getExtent2D());

		vk::ClearColorValue clearColor;
		clearColor.setFloat32({0.4,0.05,0.15,1.0});

		vk::ClearValue clearValue;
		clearValue.setColor(clearColor);

		vk::RenderPassBeginInfo bi;
		bi.setFramebuffer(frameBuffer);
		bi.setRenderPass(renderPass.getHandle());
		bi.setRenderArea(renderArea);
		bi.setPClearValues(&clearValue);
		bi.setClearValueCount(1);

		auto& pipeline = graphicsPipelines.at(pipelineName);
		commandBuffer.beginRenderPass(bi);
		commandBuffer.bindGraphicsPipeline(pipeline.getHandle());
		commandBuffer.bindDescriptorSets(
				pipeline.getLayout(),
				frame.getDescriptorSets(),
				{0});
		commandBuffer.drawVerts(3, 0);
		commandBuffer.endRenderPass();

		commandBuffer.end();
	}
}

void Renderer::render()
{
	auto& renderBuffer = beginFrame();
	assert(renderBuffer.isRecorded() && "Render buffer is not recorded");

	updateDescriptor(activeFrameIndex, fragmentInput);

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

void Renderer::setFragmentInput(FragmentInput input)
{
	fragmentInput = input;
}

CommandBuffer& Renderer::beginFrame()
{
	//to do: look into storing the imageAcquiredSemaphore in the command buffer itself
	auto& prevFrame = frames.at(activeFrameIndex);
	imageAcquiredSemaphore = prevFrame.requestSemaphore();
	activeFrameIndex = swapchain->acquireNextImage(imageAcquiredSemaphore, nullptr);
	return frames.at(activeFrameIndex).getRenderBuffer();
}

void Renderer::bindToDescription(Description& description)
{
	pDescription = &description;
	descriptionIsBound = true;
}

void Renderer::createRenderPass(std::string name, vk::Format colorFormat)
{
	renderPasses.emplace(name, RenderPass(device, colorFormat, renderPassCount));
	renderPassCount++;
}

void Renderer::createGraphicsPipeline(
		const std::string name, 
		const std::string vertShader,
		const std::string fragShader, 
		const std::string renderPassName,
		const bool geometric)
{
	vk::Rect2D renderArea;
	renderArea.setOffset({0,0});
	renderArea.setExtent({
			swapchain->getExtent3D().width, 
			swapchain->getExtent3D().height});

	std::vector<const Shader*> shaderPointers; 
	const Shader* vert = &vertexShaders.at(vertShader);
	const Shader* frag = &fragmentShaders.at(fragShader);
	shaderPointers.push_back(vert);
	shaderPointers.push_back(frag);

	auto& f = fragmentShaders.at(fragShader);

	vk::PipelineLayout layout = pipelineLayouts.at("default");
	vk::PipelineVertexInputStateCreateInfo vertexState;

	auto& renderPass = renderPasses.at(renderPassName);

	if (geometric)
	{
		assert(descriptionIsBound);

		auto attributeDescriptions = pDescription->getAttributeDescriptions();
		auto vertexBindingDescriptions = pDescription->getBindingDescriptions();
		vertexState = GraphicsPipeline::createVertexInputState(
				attributeDescriptions,
				vertexBindingDescriptions);
	}

	else 
	{
		vertexState.setPVertexBindingDescriptions(nullptr);
		vertexState.setPVertexAttributeDescriptions(nullptr);
		vertexState.setVertexBindingDescriptionCount(0);
		vertexState.setVertexAttributeDescriptionCount(0);
	}

	graphicsPipelines.emplace(name, GraphicsPipeline(
				device,
				layout,
				renderPass.getHandle(),
				0,
				renderArea,
				shaderPointers,
				vertexState));
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

	std::array<vk::DescriptorSetLayoutBinding, 3> bindings{
		uboBinding, 
		samplerBinding,
		fragmentInput};

	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.setPBindings(bindings.data());
	createInfo.setBindingCount(3);
	auto layout = device.createDescriptorSetLayout(createInfo);

	descriptorSetLayouts.insert({name, std::move(layout)}); 
	//create a default descriptor set layout presuming one ubo 
	//and one texture sampler
}

void Renderer::createPipelineLayout(
		const std::string name, 
		const std::vector<std::string> setLayoutNames)
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

	pipelineLayouts.emplace("default", device.createPipelineLayout(ci));
}

void Renderer::createDescriptorSets(
		std::vector<RenderFrame>& frames, 
		const std::vector<std::string> setLayoutNames)
{
	for (auto& frame : frames) 
	{
		std::vector<vk::DescriptorSetLayout> layouts;
		for (auto layoutName : setLayoutNames) 
		{
			layouts.push_back(descriptorSetLayouts.at(layoutName));
		}
		frame.createDescriptorSet(layouts);
	}
}

void Renderer::updateDescriptor(uint32_t frameIndex, const FragmentInput& value)
{
	auto& buffer = frames[frameIndex].getDescriptorBuffer();
	auto pHostMemory = buffer.getPHostMem();
	memcpy(pHostMemory, &value, sizeof(FragmentInput));
}

const std::string Renderer::loadShader(
		const std::string path, const std::string name, ShaderType type)
{
	if (type == ShaderType::vert)
		vertexShaders.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(name),
				std::forward_as_tuple(device, path));
	if (type == ShaderType::frag)
	{
		fragmentShaders.emplace(
				std::piecewise_construct, 
				std::forward_as_tuple(name), 
				std::forward_as_tuple(device, path));
		fragmentShaders.at(name).setWindowResolution(
				swapchain->getExtent2D().width,
				swapchain->getExtent2D().height);
	}
	return name;
}
