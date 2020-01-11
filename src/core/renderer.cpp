//storage space for now, but may be a class for 3D to 2D conversions
#include <vector>
#include "renderer.hpp"
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

RenderPass::RenderPass(const vk::Device& device, const std::string name, bool isOffscreen, uint32_t id) :
	device{device},
	id{id},
	name{name},
    offscreen{isOffscreen}
{
}

RenderPass::~RenderPass()
{
	if (handle)
		device.destroyRenderPass(handle);
}

RenderPass::RenderPass(RenderPass&& other) :
	device{other.device},
	id{other.id},
	name{other.name},
	handle{other.handle},
	attachments{std::move(other.attachments)},
	colorAttachments{std::move(other.colorAttachments)},
	references{std::move(other.references)},
	subpasses{std::move(other.subpasses)},
	subpassDependencies{std::move(other.subpassDependencies)},
    offscreen{other.offscreen}
{
	other.handle = nullptr;
	other.attachments.clear();
	other.colorAttachments.clear();
	other.references.clear();
	other.subpasses.clear();
	other.subpassDependencies.clear();
}

bool RenderPass::operator<(const RenderPass& rhs)
{
	return handle == rhs.handle;
}

void RenderPass::createColorAttachment(
		const vk::Format format,
		const vk::ImageLayout initialLayout,
		const vk::ImageLayout finalLayout,
        vk::ClearColorValue clearColor,
        vk::AttachmentLoadOp loadOp)
{
	vk::AttachmentDescription attachment;
	attachment.setFormat(format);
	attachment.setSamples(vk::SampleCountFlagBits::e1);
	//Sets what to do with data in the attachment
	//before rendering
    attachment.setLoadOp(loadOp);
	attachment.setInitialLayout(initialLayout);
	//Sets what we do with the data after rendering
	//We want to show it so we will store it
	attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
	attachment.setFinalLayout(finalLayout);

	colorAttachments.push_back(attachment);
	attachments.push_back(attachment);

	vk::AttachmentReference colorRef;
	colorRef.setAttachment(0); //our first attachment is color
	colorRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	references.push_back(colorRef);
    clearValue.setColor(clearColor);
}

void RenderPass::createBasicSubpassDependency()
{
	vk::SubpassDependency d0;
	d0.setSrcSubpass(VK_SUBPASS_EXTERNAL);
	d0.setDstSubpass(0);
	d0.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
	d0.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
	d0.setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
	d0.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	d0.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
	subpassDependencies.push_back(d0);

	vk::SubpassDependency d1;
	d1.setSrcSubpass(0);
	d1.setDstSubpass(VK_SUBPASS_EXTERNAL);
	d1.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
	d1.setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
	d1.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	d1.setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
	d1.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
	subpassDependencies.push_back(d1);
}

void RenderPass::addSubpassDependency(vk::SubpassDependency d)
{
	subpassDependencies.push_back(d);
}

void RenderPass::createSubpass()
{
	vk::SubpassDescription subpass;
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpass.setColorAttachmentCount(colorAttachments.size());
	subpass.setPColorAttachments(references.data());
	subpass.setPResolveAttachments(nullptr);
	subpass.setInputAttachmentCount(0);
	subpass.setPInputAttachments(nullptr);
	subpass.setPreserveAttachmentCount(0);
	subpass.setPPreserveAttachments(nullptr);
	subpass.setPDepthStencilAttachment(nullptr);
	subpasses.push_back(subpass);
}

void RenderPass::create()
{
	//if any of the sizes are 0, then the corresponding data
	//is undefined. but since the counts are 0, this should
	//be ok
	vk::RenderPassCreateInfo createInfo;
	createInfo.setAttachmentCount(attachments.size());
	createInfo.setPAttachments(attachments.data());
	assert(subpasses.size() != 0 && "Must be at least one subpass");
	createInfo.setSubpassCount(subpasses.size());
	createInfo.setPSubpasses(subpasses.data());
	createInfo.setDependencyCount(subpassDependencies.size());
	createInfo.setPDependencies(subpassDependencies.data());
  	handle = device.createRenderPass(createInfo);
	created = true;
}

bool RenderPass::isCreated() const
{
	return created;
}

bool RenderPass::isOffscreen() const
{
    return offscreen;
}

const vk::ClearValue* RenderPass::getClearValue() const
{
    return &clearValue;
}

const vk::RenderPass& RenderPass::getHandle() const
{
	return handle;
}

uint32_t RenderPass::getId() const
{
	return id;
}

Attachment::Attachment(
		const vk::Device& device,
		const vk::Extent2D extent) :
	device{device},
	extent{extent}
{
	vk::Extent3D ex = {extent.width, extent.height, 1};
	format = vk::Format::eR8G8B8A8Unorm;
	auto image = std::make_unique<mm::Image>(
			device, 
			ex,
			format,
			vk::ImageUsageFlagBits::eColorAttachment | 
			vk::ImageUsageFlagBits::eSampled,
			vk::ImageLayout::eUndefined);
	images.emplace_back(std::move(image));
	std::cout << "IMG " << &images.at(0) << std::endl;
}

Attachment::Attachment(const vk::Device& device, std::unique_ptr<mm::Image> swapimage) :
	device{device}
{
	extent = swapimage->getExtent2D();
	images.push_back(std::move(swapimage));
}

Attachment::~Attachment()
{
}

vk::Format Attachment::getFormat() const
{
	return format;
}

vk::Extent2D Attachment::getExtent() const
{
    return extent;
}

const mm::Image& Attachment::getImage(uint32_t index) const
{
    return *images.at(index);
}

GraphicsPipeline::GraphicsPipeline(
        const std::string name,
		const vk::Device& device, 
		const vk::PipelineLayout& layout,
		const RenderPass& renderPass,
		const uint32_t subpassIndex,
		const vk::Rect2D renderArea,
		const std::vector<const Shader*>& shaders,
		const vk::PipelineVertexInputStateCreateInfo& vertexInputState) :
    name{name},
	device{device},
	layout{layout},
	renderPass{renderPass},
    renderArea{renderArea}
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
	ci.setRenderPass(renderPass.getHandle());
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
    name{other.name},
	device{other.device},
	handle{std::move(other.handle)},
	layout{std::move(other.layout)},
	renderPass{other.renderPass},
    renderArea{other.renderArea}
{
	other.handle = nullptr;
}

const vk::Pipeline& GraphicsPipeline::getHandle() const
{
	return handle;
}

const vk::PipelineLayout& GraphicsPipeline::getLayout() const
{
	return layout;
}

const RenderPass& GraphicsPipeline::getRenderPass() const
{
	return renderPass;
}

vk::Rect2D GraphicsPipeline::getRenderArea() const
{
    return renderArea;
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
    ci.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
	ci.setDstColorBlendFactor(vk::BlendFactor::eDstAlpha);
	ci.setColorBlendOp(vk::BlendOp::eSrcOverEXT);
	ci.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
	ci.setDstAlphaBlendFactor(vk::BlendFactor::eOne);
	ci.setAlphaBlendOp(vk::BlendOp::eSrcOverEXT);
	ci.setColorWriteMask(writeMask);
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

Framebuffer::Framebuffer(
        const vk::Device& device,
        const Attachment& target, 
        const RenderPass& pass,
        const GraphicsPipeline& pipe) :
    renderTarget{target},
    renderPass{pass},
    pipeline{pipe},
    device{device}
{
    vk::FramebufferCreateInfo ci;
    ci.setWidth(target.getExtent().width);
    ci.setHeight(target.getExtent().height);
    ci.setAttachmentCount(1);
    ci.setPAttachments(&target.getImage(0).getView());
    ci.setLayers(1);
    ci.setRenderPass(pass.getHandle()); //only a template for where this fb can be used
    handle = device.createFramebuffer(ci);
}

Framebuffer::~Framebuffer()
{
    if (handle)
        device.destroyFramebuffer(handle);
}

Framebuffer::Framebuffer(Framebuffer&& other) :
    renderTarget{other.renderTarget},
    renderPass{other.renderPass},
    pipeline{other.pipeline},
    device{other.device},
    handle{std::move(other.handle)}
{
    other.handle = nullptr;
}

const RenderPass& Framebuffer::getRenderPass() const
{
    return renderPass;
}

const GraphicsPipeline& Framebuffer::getPipeline() const
{
    return pipeline;
}

const vk::Framebuffer& Framebuffer::getHandle() const
{
    return handle;
}

RenderFrame::RenderFrame(
		const Context& context, 
		std::unique_ptr<Attachment>&& renderTarget,
		uint32_t width, uint32_t height) :
	context(context),
	device(context.device),
	swapchainAttachment{std::move(renderTarget)},
	commandPool{CommandPool(
            device, 
            context.queue, 
            context.getGraphicsQueueFamilyIndex(),
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer)},
	width{width},
	height{height}
{
	vk::SemaphoreCreateInfo semaInfo;
	vk::FenceCreateInfo fenceInfo;
	semaphore = device.createSemaphore(semaInfo);
	fence = device.createFence(fenceInfo);
	createDescriptorPool();
}

RenderFrame::~RenderFrame()
{
	if (semaphore)
		device.destroySemaphore(semaphore);
	if (fence)	
		device.destroyFence(fence);
	if (descriptorPool)
		device.destroyDescriptorPool(descriptorPool);
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
	commandBuffers{std::move(other.commandBuffers)},
	swapchainAttachment{std::move(other.swapchainAttachment)},
	offscreenAttachment{std::move(other.offscreenAttachment)},
    framebuffers{std::move(other.framebuffers)},
    bufferBlock{other.bufferBlock}
{
	other.semaphore = nullptr;
	other.fence = nullptr;
	other.descriptorPool = nullptr;
    other.framebuffers.clear();
}

void RenderFrame::addFramebuffer(
        const Attachment& target, 
        const RenderPass& pass, 
        const GraphicsPipeline& pipe)
{
    framebuffers.emplace_back(
                device,
                target,
                pass,
                pipe);
}

void RenderFrame::addFramebuffer(
        TargetType type,
        const RenderPass& pass, 
        const GraphicsPipeline& pipe)
{
    if (type == TargetType::offscreen)
        framebuffers.emplace_back(
                    device,
                    *offscreenAttachment,
                    pass,
                    pipe);
    if (type == TargetType::swapchain)
        framebuffers.emplace_back(
                    device,
                    *swapchainAttachment,
                    pass,
                    pipe);
    else
        throw std::runtime_error("invalid target type");
}

void RenderFrame::clearFramebuffers()
{
    framebuffers.clear();
}

CommandBuffer& RenderFrame::requestRenderBuffer(uint32_t bufferId)
{
    if (bufferId < commandBuffers.size())
    {
        auto& buffer = *commandBuffers.at(bufferId);
        buffer.reset();
        return buffer;
    }
    else
    {
        auto cmdPtr = &commandPool.requestCommandBuffer();
        commandBuffers.push_back(cmdPtr);
        return *commandBuffers.back();
    }
}

CommandBuffer& RenderFrame::getRenderBuffer(uint32_t bufferId)
{
    return *commandBuffers.at(bufferId);
}

vk::Semaphore RenderFrame::requestSemaphore()
{
	return semaphore;
}

void RenderFrame::createDescriptorPool()
{
	vk::DescriptorPoolSize uboDynamicSize;
	vk::DescriptorPoolSize imageSamplerSize;
	vk::DescriptorPoolSize uboSize;
	uboDynamicSize.setType(vk::DescriptorType::eUniformBufferDynamic);
	uboDynamicSize.setDescriptorCount(1);
	imageSamplerSize.setType(vk::DescriptorType::eCombinedImageSampler);
	imageSamplerSize.setDescriptorCount(20); //arbitrary
	uboSize.setType(vk::DescriptorType::eUniformBuffer);
	uboSize.setDescriptorCount(1);

	std::array<vk::DescriptorPoolSize, 3> sizes{
		uboSize, imageSamplerSize, uboDynamicSize};

	vk::DescriptorPoolCreateInfo ci;
	ci.setMaxSets(1); //currently allow just one descriptor set
	ci.setPPoolSizes(sizes.data());
	ci.setPoolSizeCount(sizes.size());

	descriptorPool = device.createDescriptorPool(ci);
}

void RenderFrame::addOffscreenAttachment(std::unique_ptr<Attachment>&& target)
{
	offscreenAttachment = std::move(target);
}

void RenderFrame::createDescriptorSets(
        const std::vector<vk::DescriptorSetLayout>& layouts)
{
	vk::DescriptorSetAllocateInfo ai;
	ai.setPSetLayouts(layouts.data());
	ai.setDescriptorSetCount(layouts.size());
	ai.setDescriptorPool(descriptorPool);
	descriptorSets = device.allocateDescriptorSets(ai);
}

const std::vector<vk::DescriptorSet>& RenderFrame::getDescriptorSets() const
{
	return descriptorSets;
}

Renderer::Renderer(Context& context, XWindow& window) :
	context(context),
	device(context.device),
	graphicsQueue(context.queue)
{
	swapchain = std::make_unique<Swapchain>(context, window, 3); 
	//we hardcode 3 swap images
	for (auto& imageHandle : swapchain->getImages()) 
	{
		auto image = std::make_unique<mm::Image>(
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
		auto offscreenAttachment{
			std::make_unique<Attachment>(
					device,
					swapchain->getExtent2D())};
		renderFrame.addOffscreenAttachment(std::move(offscreenAttachment));
		frames.emplace_back(std::move(renderFrame));
	}
    createDescriptorPool();
    createDescriptorBuffer(2000); //arbitrary for now
    fragUBOs.resize(1);
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

RenderPass& Renderer::createRenderPass(std::string name, bool offscreen)
{
	auto renderPass{RenderPass(device, name, offscreen, renderPassCount)};
	renderPasses.emplace(name, std::move(renderPass));
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

void Renderer::initFrameUBOs(uint32_t binding)
{
    for (auto& frame : frames) 
    {
        auto block = descriptorBuffer->requestBlock(sizeof(FragmentInput));
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
    descriptorBuffer->map();
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

void Renderer::updateFrameSamplers(const std::vector<const mm::Image*> images, uint32_t binding)
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

void Renderer::prepareAsOffscreenPass(RenderPass& offScreenPass)
{
    vk::ClearColorValue cv;
    cv.setFloat32({.0,.0,.0,0.});
	offScreenPass.createColorAttachment(
			vk::Format::eR8G8B8A8Unorm,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eShaderReadOnlyOptimal,
            cv,
            vk::AttachmentLoadOp::eLoad);
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

Attachment& Renderer::createAttachment(const std::string name, const vk::Extent2D extent)
{
    auto attachment = std::make_unique<Attachment>(device, extent);
    attachments.emplace(name, std::move(attachment));
    return *attachments.at(name);
}

GraphicsPipeline& Renderer::createGraphicsPipeline(
		const std::string name, 
        const std::string pipelineLayout,
		const VertShader& vertShader,
		const FragShader& fragShader,
		const RenderPass& renderPass,
		const bool geometric)
{
	vk::Rect2D renderArea;
	renderArea.setOffset({0,0});
	renderArea.setExtent({
			swapchain->getExtent3D().width, 
			swapchain->getExtent3D().height});

	std::vector<const Shader*> shaderPointers = {&vertShader, &fragShader};

	vk::PipelineLayout layout = pipelineLayouts.at(pipelineLayout);

    vk::PipelineVertexInputStateCreateInfo vertexState;
	if (geometric)
	{
		assert(descriptionIsBound);

		auto attributeDescriptions = pDescription->getAttributeDescriptions();
		auto vertexBindingDescriptions = pDescription->getBindingDescriptions();
		vertexState = GraphicsPipeline::createVertexInputState(
				attributeDescriptions, vertexBindingDescriptions);
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
				renderPass,
				0,
				renderArea,
				shaderPointers,
				vertexState));

    return graphicsPipelines.at(name);
}

VertShader& Renderer::loadVertShader(
        const std::string path, const std::string name)
{
    vertexShaders.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(device, path));
    return vertexShaders.at(name);
}

FragShader& Renderer::loadFragShader(
		const std::string path, const std::string name)
{
    fragmentShaders.emplace(
            std::piecewise_construct, 
            std::forward_as_tuple(name), 
            std::forward_as_tuple(device, path));
    fragmentShaders.at(name).setWindowResolution(
            swapchain->getExtent2D().width,
            swapchain->getExtent2D().height);
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
            auto& framebuffer = frame.framebuffers[fbId];
            auto& renderPass = framebuffer.getRenderPass();
            auto& pipeline = framebuffer.getPipeline();

            vk::RenderPassBeginInfo bi;
            bi.setFramebuffer(framebuffer.getHandle());
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

void Renderer::addFramebuffer(
        const Attachment& attachment, 
        const RenderPass& rpass, 
        const GraphicsPipeline& pipe)
{
    for (auto& frame : frames) 
    {
        frame.addFramebuffer(attachment, rpass, pipe);
    }
}

void Renderer::addFramebuffer(
        const TargetType type,
        const RenderPass& rpass, 
        const GraphicsPipeline& pipe)
{
    for (auto& frame : frames) 
    {
        frame.addFramebuffer(type, rpass, pipe);
    }
}

void Renderer::clearFramebuffers()
{
    for (auto& frame : frames) 
    {
        frame.clearFramebuffers();
    }
}

//void Renderer::prepare(const std::string path)
//{
//
//    //create paint render target
//    auto target0 = std::make_unique<Attachment>(device, swapchain->getExtent2D());
//    layers.insert({"target0", std::move(target0)});
//    activeTarget = layers.at("target0").get();
//
//	//the following code will handle descriptor set creation
//	//while we don't need descriptor sets to create a Pipeline
//	//we do need to make the Pipeline aware of their layouts 
//	//if we do intend to use them
//	//----------------------------
//	createDefaultDescriptorSetLayout("default");
//	//we can now create the desriptorsets for the render frames 
//	//additionally we can create our pipeline layouts
//	createPipelineLayout("default", {"default"});
//
//	//note: this could have been done as soon as we had the layout
//	createDescriptorSets(frames, {"default"});
//	//----------------------------
//	//
//	//we'll create renderpass now. 
//	//note: we could have done this at any point
//    
//    for (auto& frame : frames) 
//    {
//        
//    }
//
//    //load shaders
//	auto& quadShader = loadVertShader("shaders/tarot/fullscreen_tri.spv", "vert1");
//	auto& tarotShader = loadFragShader(path, "frag1");
//	auto& blurShader = loadFragShader("shaders/radialBlur.spv", "osFrag");
//
//    //create render passes
//    auto& offScreenPass = createRenderPass("offscreen", true);
//    prepareAsOffscreenPass(offScreenPass); //can make this a static function of RenderPass
//    auto& swapchainPass = createRenderPass("swapchain", false); 
//    prepareAsSwapchainPass(swapchainPass); //can make this a static function of RenderPass
//    
//    //create pipelines
//	auto& offscreenPipe = createGraphicsPipeline(
//            "offscreen", quadShader, tarotShader, offScreenPass, false);
//	auto& swapchainPipe = createGraphicsPipeline(
//            "default", quadShader, blurShader, swapchainPass, false);
//    
//    //create framebuffers
//    for (auto& frame : frames) 
//    {
//        frame.addFramebuffer(*activeTarget, offScreenPass, offscreenPipe);
//        frame.addFramebuffer(TargetType::swapchain, swapchainPass, swapchainPipe);
//    }
//}

void Renderer::render(uint32_t cmdId)
{
	auto& renderBuffer = beginFrame(cmdId);
	assert(renderBuffer.isRecorded() && "Render buffer is not recorded");

	updateFrameDescriptorBuffer(activeFrameIndex, fragUBOs[0]);

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

void Renderer::setFragmentInput(uint32_t index, FragmentInput input)
{
    fragUBOs.at(index) = input;
}

CommandBuffer& Renderer::beginFrame(uint32_t cmdId)
{
	//to do: look into storing the imageAcquiredSemaphore in the command buffer itself
	auto& prevFrame = frames.at(activeFrameIndex);
	imageAcquiredSemaphore = prevFrame.requestSemaphore();
	activeFrameIndex = swapchain->acquireNextImage(imageAcquiredSemaphore, nullptr);
	return frames.at(activeFrameIndex).getRenderBuffer(cmdId);
}

void Renderer::bindToDescription(Description& description)
{
	pDescription = &description;
	descriptionIsBound = true;
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

void Renderer::updateFrameDescriptorBuffer(uint32_t frameIndex, const FragmentInput& value)
{
	auto block = descriptorBuffer->bufferBlocks[frameIndex].get();
    assert(block->isMapped && "Block not mapped!");
	auto pHostMemory = block->pHostMemory;
    std::cout << "pHostMemory before copy " << pHostMemory << std::endl;
	memcpy(pHostMemory, &value, sizeof(FragmentInput));
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
	descriptorBuffer = std::make_unique<mm::Buffer>(
			device, 
            context.physicalDeviceProperties,
			context.physicalDeviceMemoryProperties, 
			size, 
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | 
			vk::MemoryPropertyFlagBits::eHostCoherent);
}
