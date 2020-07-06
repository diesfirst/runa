#ifndef RENDER_PIPELINE_HPP
#define RENDER_PIPELINE_HPP

//imp: "pipeline.cpp"

#include <types/vktypes.hpp>

namespace sword
{

namespace render
{

class RenderPass;
class Shader;

class GraphicsPipeline
{
public:
    GraphicsPipeline(
        const std::string name,
        const vk::Device&, 
        const vk::PipelineLayout&, 
        const RenderPass&,
        const uint32_t subpassIndex,
        const vk::Rect2D, //we could choose to make this dynamic
        const std::vector<const Shader*>,
        const vk::PipelineVertexInputStateCreateInfo, 
        const vk::PolygonMode);
    ~GraphicsPipeline() = default;
    GraphicsPipeline(GraphicsPipeline&&) = default;

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;

    void create();
    void recreate();
    vk::Viewport createViewport(const vk::Rect2D&);
    const vk::Pipeline& getHandle() const;
    const vk::PipelineLayout& getLayout() const;
    const RenderPass& getRenderPass() const;
    vk::Rect2D getRenderArea() const;
    std::vector<vk::PipelineColorBlendAttachmentState> attachmentStates;
    bool isCreated() const;
    vk::PipelineRasterizationStateCreateInfo rasterizationState;

    static vk::PipelineVertexInputStateCreateInfo createVertexInputState(
    const std::vector<vk::VertexInputAttributeDescription>&,
    const std::vector<vk::VertexInputBindingDescription>&);

private:
    const std::string name;
    vk::UniquePipeline handle;
    const vk::Device& device;
    const RenderPass& renderPass;
    const vk::PipelineLayout& layout;
    vk::Rect2D renderArea;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;
    vk::PipelineViewportStateCreateInfo viewportState;
    vk::PipelineMultisampleStateCreateInfo multisampleState;
    vk::PipelineColorBlendStateCreateInfo colorBlendState;
    vk::PipelineDepthStencilStateCreateInfo depthStencilState;
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblySate;
    vk::PipelineVertexInputStateCreateInfo vertexInputState;
    const std::vector<const Shader*> shaders;
    vk::Viewport viewport;
    vk::Rect2D scissor;
    vk::PolygonMode polygonMode;
    uint32_t subpassIndex{0};
    bool created{false};

    std::vector<vk::PipelineShaderStageCreateInfo> extractShaderStageInfos();
    vk::PipelineViewportStateCreateInfo createViewportState();
    vk::PipelineRasterizationStateCreateInfo createRasterizationState();
    vk::PipelineMultisampleStateCreateInfo createMultisampleState();
    vk::PipelineColorBlendAttachmentState createColorBlendAttachmentState();
    vk::PipelineColorBlendStateCreateInfo createColorBlendState();
    vk::PipelineDepthStencilStateCreateInfo createDepthStencilState();
    vk::PipelineInputAssemblyStateCreateInfo createInputAssemblyState();
};

}; // namespace render

}; // namespace sword

#endif /* end of include guard: RENDER_PIPELINE_HPP */
