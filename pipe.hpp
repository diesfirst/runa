#ifndef PIPE_H
#define PIPE_H

#include "context.hpp"
#include "renderer.hpp"

class Pipe
{
public:
	Pipe(const Context&);
	virtual ~Pipe();
	void createGraphicsPipeline(const Renderer&);

	vk::Pipeline graphicsPipeline;

private:
	const Context& context;
	uint32_t width, height;
	vk::Viewport viewport;
	vk::Rect2D scissor;
	vk::PipelineLayout pipelineLayout;
	vk::DynamicState dynamicStates; //not used right now
	vk::PipelineColorBlendAttachmentState colorAttachmentState; //could be many
	vk::PipelineColorBlendStateCreateInfo colorBlending;
	vk::PipelineRasterizationStateCreateInfo rasterizer;
	vk::PipelineMultisampleStateCreateInfo multisampling;
	vk::PipelineVertexInputStateCreateInfo vertexInputState;
	vk::PipelineViewportStateCreateInfo viewportState;
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
	
	vk::ShaderModule createShaderModule(const std::vector<char>& code);
	void initVertexInputState();
	void initInputAssemblyState();
	void initViewportState();
	void initRasterizer();
	void initMultisampling();
	void initColorAttachment();
	void initColorBlending();
	void initPipelineLayout();
};

#endif /* PIPE_H */
