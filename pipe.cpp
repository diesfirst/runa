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
	initDescriptorSetLayout();
	initPipelineLayout();
}

Pipe::~Pipe()
{
	context.device.destroyPipelineLayout(pipelineLayout);
	context.device.destroyDescriptorSetLayout(descriptorSetLayout);
	context.device.destroyDescriptorPool(descriptorPool);
	context.device.destroyPipeline(graphicsPipeline);
}

void Pipe::createGraphicsPipeline(const Renderer& renderer, const uint32_t width, const uint32_t height)
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
	inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleStrip);
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
	rasterizer.setPolygonMode(vk::PolygonMode::eLine);
	rasterizer.setLineWidth(1.0); //in units of fragments
	rasterizer.setCullMode(vk::CullModeFlagBits::eNone);
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
	info.setSetLayoutCount(1);
	info.setPSetLayouts(&descriptorSetLayout);
	pipelineLayout = context.device.createPipelineLayout(info);
}

void Pipe::initDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding binding;
	binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
	binding.setBinding(0);
	//possible to use an array of uniform buffer objects
	//could be used for skeletal animation
	binding.setDescriptorCount(1);
	binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
	binding.setPImmutableSamplers(nullptr);

	vk::DescriptorSetLayoutCreateInfo info;
	info.setPBindings(&binding);
	info.setBindingCount(1);
	descriptorSetLayout = context.device.createDescriptorSetLayout(info);
}

void Pipe::createDescriptorPool(uint32_t descriptorCount)
{
	vk::DescriptorPoolSize poolSize;
	poolSize.setType(vk::DescriptorType::eUniformBuffer);
	poolSize.setDescriptorCount(descriptorCount);

	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.setPoolSizeCount(1);
	poolInfo.setPPoolSizes(&poolSize);
	poolInfo.setMaxSets(descriptorCount);

	descriptorCount = descriptorCount;
	descriptorPool = context.device.createDescriptorPool(poolInfo);
}

void Pipe::createDescriptorSets(uint32_t count)
{
	std::vector<vk::DescriptorSetLayout> layouts(count, descriptorSetLayout);

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.setDescriptorPool(descriptorPool);
	allocInfo.setDescriptorSetCount(count);
	allocInfo.setPSetLayouts(layouts.data());

	descriptorSets.resize(count);
	descriptorSets = context.device.allocateDescriptorSets(allocInfo);
}

void Pipe::updateDescriptorSets(
			uint32_t count, 
			std::vector<bufferBlock>* uboBlocks,
			size_t uboSize)
{
	assert(descriptorsPrepared);

	vk::DescriptorBufferInfo bufferInfo;
	bufferInfo.setOffset(0);
	bufferInfo.setRange(uboSize);

	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
	descriptorWrite.setDescriptorCount(1);
	descriptorWrite.setDstBinding(0);
	descriptorWrite.setDstArrayElement(0);
	descriptorWrite.setPBufferInfo(&bufferInfo);
	descriptorWrite.setPImageInfo(nullptr);
	descriptorWrite.setPTexelBufferView(nullptr);

	for (int i = 0; i < count; ++i) 
	{
		bufferInfo.setBuffer((*uboBlocks)[i].buffer);
		descriptorWrite.setDstSet(descriptorSets[i]);
		context.device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
	}
}

void Pipe::prepareDescriptors(uint32_t count)
{
	initDescriptorSetLayout();
	createDescriptorPool(count);
	createDescriptorSets(count);
	descriptorsPrepared = true;
}

