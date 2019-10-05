//storage space for now, but may be a class for 3D to 2D conversions
#include <vector>
#include "renderer.hpp"
#include "viewport.hpp"
#include "io.hpp"
#include <iostream>
#include "description.hpp"

std::vector<vk::PipelineShaderStageCreateInfo> createShaderStageInfos(
		vk::ShaderModule& vertModule,
		vk::ShaderModule& fragModule)
{
	vk::PipelineShaderStageCreateInfo vertInfo;
	vk::PipelineShaderStageCreateInfo fragInfo;
	vertInfo.setStage(vk::ShaderStageFlagBits::eVertex);
	vertInfo.setModule(vertModule);
	vertInfo.setPName("main");
	fragInfo.setStage(vk::ShaderStageFlagBits::eFragment);
	fragInfo.setModule(fragModule);
	fragInfo.setPName("main");
	return std::vector<vk::PipelineShaderStageCreateInfo>({vertInfo, fragInfo});
}


Renderer::Renderer(
		const Context& context) :
	context(context)
{
	viewportIsBound = false;
	descriptionIsBound = false;
	createRenderPass(vk::Format::eB8G8R8A8Unorm); //by inspection
	initAll();
}

Renderer::~Renderer()
{
	context.device.destroyPipelineLayout(pipelineLayout);
	context.device.destroyPipeline(graphicsPipeline);
	context.device.destroyRenderPass(renderPass);
	for (auto framebuffer : framebuffers) 
	{
		context.device.destroyFramebuffer(framebuffer);
	}
}

void Renderer::bindToViewport(Viewport& viewport)
{
	pViewport = &viewport;
	width = viewport.getWidth();
	height = viewport.getHeight();
	viewportIsBound = true;
	createFramebuffers();
}

void Renderer::bindToDescription(Description& description)
{
	pDescription = &description;
	descriptionIsBound = true;
}

void Renderer::setup(Viewport& viewport, Description& description)
{
	bindToViewport(viewport);
	bindToDescription(description);
	description.createCamera(viewport.getWidth(), viewport.getHeight());
	description.prepareUniformBuffers(viewport.getSwapImageCount());
	description.prepareDescriptorSets(viewport.getSwapImageCount());
	initPipelineLayout();
	createGraphicsPipeline();
}

void Renderer::createRenderPass(vk::Format colorFormat)
{
	std::vector<vk::AttachmentDescription> attachments;
	std::vector<vk::AttachmentReference> references;
	std::vector<vk::SubpassDescription> subpasses;
	std::vector<vk::SubpassDependency> subpassDependencies;

	vk::AttachmentDescription attachment;
	attachment.setFormat(colorFormat);
	attachment.setSamples(vk::SampleCountFlagBits::e1);
	//Sets what to do with data in the attachment
	//before rendering
	attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
	//Sets what we do with the data after rendering
	//We want to show it so we will store it
	attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
	attachment.setInitialLayout(vk::ImageLayout::eUndefined);
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
	dependency.setSrcAccessMask(
			vk::AccessFlagBits::eColorAttachmentWrite);
	dependency.setSrcStageMask(
			vk::PipelineStageFlagBits::eColorAttachmentOutput);
	dependency.setDstSubpass(
			0);
	dependency.setDstAccessMask(
			vk::AccessFlagBits::eColorAttachmentRead);
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
  renderPass = context.device.createRenderPass(createInfo);
}


void Renderer::createFramebuffers()
{
	if (!viewportIsBound)
		throw std::runtime_error("Viewport not bound!");
	vk::FramebufferCreateInfo createInfo;
	createInfo.setWidth(width);
	createInfo.setHeight(height);
	std::cout << "width: " << width << std::endl;
	std::cout << "height: " << height << std::endl;
	createInfo.setRenderPass(renderPass); 
	createInfo.setAttachmentCount(1);
	createInfo.setLayers(1);
	std::array<vk::ImageView, 1> imageViewsTemp;
	for (int i = 0; i < pViewport->getSwapImageCount(); ++i)
	{
		imageViewsTemp[0] = pViewport->getSwapImageView(i);
		createInfo.setPAttachments(imageViewsTemp.data());
		framebuffers.push_back(
				context.device.createFramebuffer(createInfo));
	}
}

void Renderer::update()
{
	vk::Buffer& vertBuffer = pDescription->getVkVertexBuffer();
	context.pCommander->recordDrawVert(
			renderPass,
			framebuffers,
			vertBuffer,
			graphicsPipeline,
			pipelineLayout,
			pDescription->descriptorSets,
			width, height,
			pDescription->getVertexCount(), pDescription->getGeoCount(), 
			pDescription->getDynamicAlignment());
}

void Renderer::render()
{
	pDescription->updateAllCurrentUbos();
	uint8_t curIndex = context.pCommander->renderFrame(pViewport->getSwapchain());
	pDescription->setCurrentSwapIndex(curIndex);
}

vk::ShaderModule Renderer::createShaderModule(const std::vector<char>& code)
{
	vk::ShaderModuleCreateInfo info;
	info.setCodeSize(code.size());
	info.setPCode(reinterpret_cast<const uint32_t*>(code.data()));
	return context.device.createShaderModule(info);
}

void Renderer::initVertexInputState()
{
}

void Renderer::initInputAssemblyState()
{
	inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);
	inputAssemblyState.setPrimitiveRestartEnable(false);
}

void Renderer::initRasterizer()
{
	//changing any of these may require enabling certain GPU features
	//rasterizer has more possible settings but i left them at default
	rasterizer.setDepthClampEnable(false);
	rasterizer.setRasterizerDiscardEnable(false);
	rasterizer.setPolygonMode(vk::PolygonMode::eFill);
	rasterizer.setLineWidth(1.0); //in units of fragments
	rasterizer.setCullMode(vk::CullModeFlagBits::eNone);
	rasterizer.setFrontFace(vk::FrontFace::eClockwise);
	rasterizer.setDepthBiasEnable(false);
}

void Renderer::initMultisampling()
{
	//this will be useful, but disabled for now
	//many settings left at default
	multisampling.setSampleShadingEnable(false);
	multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
}

void Renderer::initColorAttachment()
{
	colorAttachmentState.setBlendEnable(false);
	colorAttachmentState.setColorWriteMask(
			vk::ColorComponentFlagBits::eA |
			vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB);
}

void Renderer::initColorBlending()
{
	colorBlending.setLogicOpEnable(false);
	colorBlending.setAttachmentCount(1);
	colorBlending.setPAttachments(&colorAttachmentState);
}

void Renderer::initPipelineLayout()
{
	vk::PipelineLayoutCreateInfo info;
	if (descriptionIsBound)
	{
		std::cout << "Setting layout according to description at "
			<< pDescription
		 	<< std::endl;
		info.setSetLayoutCount(1);
		info.setPSetLayouts(pDescription->getPDescriptorSetLayout());
		pipelineLayout = context.device.createPipelineLayout(info);
	}
	else
	{
		std::cout << "Setting empty layout" << std::endl;
		pipelineLayout = context.device.createPipelineLayout(info);
	}
}

void Renderer::initAll()
{
	initInputAssemblyState();
	initRasterizer();
	initMultisampling();
	initColorAttachment();
	initColorBlending();
}

void Renderer::createGraphicsPipeline()
{
	auto vertShaderCode = io::readFile("shaders/vert.spv");
	auto fragShaderCode = io::readFile("shaders/frag.spv");
	vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	auto shaderStages = createShaderStageInfos(
			vertShaderModule, fragShaderModule);
	
	auto bindingDescription = Description::getBindingDescription();
	auto attributeDescriptions  = Description::getAttributeDescriptions();

	vertexInputState.setVertexBindingDescriptionCount(1);
	vertexInputState.setVertexAttributeDescriptionCount(
			attributeDescriptions.size());
	vertexInputState.setPVertexBindingDescriptions(&bindingDescription);
	vertexInputState.setPVertexAttributeDescriptions(
			attributeDescriptions.data());

	vk::GraphicsPipelineCreateInfo info;
	info.setStageCount(2);
	info.setPStages(shaderStages.data());
	info.setPVertexInputState(&vertexInputState);
	info.setPInputAssemblyState(&inputAssemblyState);
	info.setPViewportState(pViewport->getPViewportState());
	info.setPRasterizationState(&rasterizer);
	info.setPMultisampleState(&multisampling);
	info.setPColorBlendState(&colorBlending);
	info.setLayout(pipelineLayout);
	info.setRenderPass(renderPass);
	info.setSubpass(0);
	//not used
	info.setPDepthStencilState(nullptr);
	info.setPDynamicState(nullptr);
	info.setBasePipelineIndex(-1);
	
	graphicsPipeline = context.device.createGraphicsPipeline({}, info);

	context.device.destroyShaderModule(vertShaderModule);
	context.device.destroyShaderModule(fragShaderModule);
}

void Renderer::destroyFramebuffers()
{
	for (auto framebuffer : framebuffers) {
		context.device.destroyFramebuffer(framebuffer);
	}
}
