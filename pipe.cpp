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
}

Pipe::~Pipe()
{
}

void Pipe::createGraphicsPipeline()
{
	auto vertShaderCode = io::readFile("shaders/vert.spv");
	auto fragShaderCode = io::readFile("shaders/frag.spv");
	vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	auto shaderStageInfos = createShaderStageInfos(
			vertShaderModule, fragShaderModule);

	vk::PipelineVertexInputStateCreateInfo vertInputStageInfo;
	vertInputStageInfo.setVertexBindingDescriptionCount(0);
	vertInputStageInfo.setVertexAttributeDescriptionCount(0);

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList);
	inputAssemblyInfo.setPrimitiveRestartEnable(false);

	vk::Viewport viewport(0, 0, width, height, 0.0, 1.0);
	vk::Rect2D scissor;
	scissor.setOffset({0,0});
	scissor.setExtent({width, height});

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
