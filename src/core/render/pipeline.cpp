#include <render/pipeline.hpp>
#include <render/renderpass.hpp>
#include <render/shader.hpp>
#include <iostream>

namespace sword
{

namespace render
{

GraphicsPipeline::GraphicsPipeline(
        const std::string name,
		const vk::Device& device, 
		const vk::PipelineLayout& layout,
		const RenderPass& renderPass,
		const uint32_t subpassIndex,
		const vk::Rect2D renderArea,
		const std::vector<const Shader*> shaders,
		const vk::PipelineVertexInputStateCreateInfo vertexInputState) :
    name{name},
	device{device},
	layout{layout},
	renderPass{renderPass},
    renderArea{renderArea},
    shaders{shaders},
    subpassIndex{subpassIndex},
    vertexInputState{vertexInputState},
    scissor{renderArea}
{
	viewport = createViewport(renderArea);
	auto attachmentState0 = createColorBlendAttachmentState();
	attachmentStates.push_back(attachmentState0);

    //none of these hold pointers so they can be created here (before any move constructors)
    rasterizationState = createRasterizationState();
    inputAssemblySate = createInputAssemblyState();
    depthStencilState = createDepthStencilState(); //not ready yet
    multisampleState = createMultisampleState();
    create();
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
    renderArea{other.renderArea},
    shaderStageInfos{std::move(other.shaderStageInfos)},
    viewportState{other.viewportState},
    multisampleState{other.multisampleState},
    rasterizationState{other.rasterizationState},
    colorBlendState{other.colorBlendState},
    depthStencilState{other.depthStencilState},
    inputAssemblySate{other.inputAssemblySate},
    shaders{other.shaders},
    attachmentStates{other.attachmentStates},
    viewport{other.viewport},
    scissor{other.scissor},
    vertexInputState{other.vertexInputState},
    created{other.created}
{
	other.handle = nullptr;
}

void GraphicsPipeline::create()
{
    viewportState = createViewportState();
    shaderStageInfos = extractShaderStageInfos();
    colorBlendState = createColorBlendState();

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
    created = true;
}

void GraphicsPipeline::recreate()
{
    device.destroy(handle);
    std::cout << "GraphicsPipeline: destroyed handle, calling create..." << '\n';
    create();
    std::cout << "GraphicsPipeline: recreation successful" << '\n';
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

bool GraphicsPipeline::isCreated() const
{
    return created;
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

vk::PipelineViewportStateCreateInfo GraphicsPipeline::createViewportState()
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

vk::PipelineColorBlendStateCreateInfo GraphicsPipeline::createColorBlendState()
{
	vk::PipelineColorBlendStateCreateInfo ci;
	ci.setLogicOpEnable(false);
	ci.setPAttachments(attachmentStates.data());
	ci.setAttachmentCount(attachmentStates.size());
	ci.setLogicOp(vk::LogicOp::eCopy);
	ci.setBlendConstants({0,0,0,0});

    vk::PipelineColorBlendAdvancedStateCreateInfoEXT advancedInfo; //can mess with this
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

std::vector<vk::PipelineShaderStageCreateInfo> GraphicsPipeline::extractShaderStageInfos()
{
	std::vector<vk::PipelineShaderStageCreateInfo> stageInfos;
	stageInfos.resize(shaders.size());
	for (int i = 0; i < shaders.size(); i++) 
	{
		stageInfos.at(i) = shaders.at(i)->getStageInfo();
	}
	return stageInfos;
}

}; // namespace render

}; // namespace sword
