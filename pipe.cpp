#include "pipe.hpp"
#include "io.hpp"
#include <iostream>

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

Pipe::Pipe(const Context& context) :
	context(context)
{

//	initVertexInputState();
	initInputAssemblyState();
	initViewportState();
	initRasterizer();
	initMultisampling();
	initColorAttachment();
	initColorBlending();
	initPipelineLayout();
}

Pipe::~Pipe()
{
	context.device.destroyPipelineLayout(pipelineLayout);
	context.device.destroyPipeline(graphicsPipeline);
}

void Pipe::createGraphicsPipeline(const Renderer& renderer, uint32_t width, uint32_t height)
{
	auto vertShaderCode = io::readFile("shaders/vert.spv");
	auto fragShaderCode = io::readFile("shaders/frag.spv");
	vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	auto shaderStages = createShaderStageInfos(
			vertShaderModule, fragShaderModule);
	
	auto bindingDescription = Geo::getBindingDescription();
	auto attributeDescriptions  = Geo::getAttributeDescriptions();

	vertexInputState.setVertexBindingDescriptionCount(1);
	vertexInputState.setVertexAttributeDescriptionCount(
			attributeDescriptions.size());
	vertexInputState.setPVertexBindingDescriptions(&bindingDescription);
	vertexInputState.setPVertexAttributeDescriptions(attributeDescriptions.data());

	viewport = vk::Viewport(0, 0, width, height, 0, 1);
	scissor.setExtent({width, height});
	scissor.setOffset({0, 0});

	vk::GraphicsPipelineCreateInfo info;
	info.setStageCount(2);
	info.setPStages(shaderStages.data());
	info.setPVertexInputState(&vertexInputState);
	info.setPInputAssemblyState(&inputAssemblyState);
	info.setPViewportState(&viewportState);
	info.setPRasterizationState(&rasterizer);
	info.setPMultisampleState(&multisampling);
	info.setPColorBlendState(&colorBlending);
	info.setLayout(pipelineLayout);
	info.setRenderPass(renderer.renderPass);
	info.setSubpass(0);
	//not used
	info.setPDepthStencilState(nullptr);
	info.setPDynamicState(nullptr);
	info.setBasePipelineIndex(-1);
	
	graphicsPipeline = context.device.createGraphicsPipeline({}, info);

	context.device.destroyShaderModule(vertShaderModule);
	context.device.destroyShaderModule(fragShaderModule);


}

vk::ShaderModule Pipe::createShaderModule(const std::vector<char>& code)
{
	vk::ShaderModuleCreateInfo info;
	info.setCodeSize(code.size());
	info.setPCode(reinterpret_cast<const uint32_t*>(code.data()));
	return context.device.createShaderModule(info);
}

void Pipe::initVertexInputState()
{
}

void Pipe::initInputAssemblyState()
{
	inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);
	inputAssemblyState.setPrimitiveRestartEnable(false);

}

void Pipe::initViewportState()
{
	viewportState.setPScissors(&scissor);
	viewportState.setPViewports(&viewport);
	viewportState.setScissorCount(1); //scissor might not be necessary
	viewportState.setViewportCount(1);
}

void Pipe::initRasterizer()
{
	//changing any of these may require enabling certain GPU features
	//rasterizer has more possible settings but i left them at default
	rasterizer.setDepthClampEnable(false);
	rasterizer.setRasterizerDiscardEnable(false);
	rasterizer.setPolygonMode(vk::PolygonMode::eFill);
	rasterizer.setLineWidth(1.0); //in units of fragments
	rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
	rasterizer.setFrontFace(vk::FrontFace::eClockwise);
	rasterizer.setDepthBiasEnable(false);
}

void Pipe::initMultisampling()
{
	//this will be useful, but disabled for now
	//many settings left at default
	multisampling.setSampleShadingEnable(false);
	multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
}

void Pipe::initColorAttachment()
{
	colorAttachmentState.setBlendEnable(false);
	colorAttachmentState.setColorWriteMask(
			vk::ColorComponentFlagBits::eA |
			vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB);
}

void Pipe::initColorBlending()
{
	colorBlending.setLogicOpEnable(false);
	colorBlending.setAttachmentCount(1);
	colorBlending.setPAttachments(&colorAttachmentState);
}

void Pipe::initPipelineLayout()
{
	vk::PipelineLayoutCreateInfo info;
	pipelineLayout = context.device.createPipelineLayout(info);
}
