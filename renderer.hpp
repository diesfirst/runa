#ifndef RENDERER_H
#define RENDERER_H

#include "context.hpp"

class Viewport;
class Swapchain;

class Renderer
{
public:
	Renderer(const Context&);
	~Renderer();
	void createRenderPass(vk::Format);
  void createFramebuffers();
	void bindToViewport(Viewport& viewport);
	void createGraphicsPipeline();

private:
	const Context& context;
	Viewport* pViewport;
	uint32_t width;
	uint32_t height;
	vk::Rect2D scissor;
	bool viewportIsBound;
	std::vector<vk::Framebuffer> framebuffers;
	vk::RenderPass renderPass;

	vk::DynamicState dynamicStates; //not used right now
	vk::PipelineColorBlendAttachmentState colorAttachmentState; //could be many
	vk::PipelineColorBlendStateCreateInfo colorBlending;
	vk::PipelineRasterizationStateCreateInfo rasterizer;
	vk::PipelineMultisampleStateCreateInfo multisampling;
	vk::PipelineVertexInputStateCreateInfo vertexInputState;
	vk::PipelineViewportStateCreateInfo viewportState;
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
	
	void destroyFramebuffers();
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

#endif /* RENDERER_H */
