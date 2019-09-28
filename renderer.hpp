#ifndef RENDERER_H
#define RENDERER_H

#include "context.hpp"
#include <memory>

class Viewport;
class Swapchain;
class Description;

class Renderer
{
public:
	Renderer(const Context&);
	~Renderer();
	void createRenderPass(vk::Format);
  void createFramebuffers();
	void bindToViewport(Viewport& viewport);
	void bindToDescription(Description& description);
	void setup(Viewport&, Description&);
	void update();
	void render();

private:
	const Context& context;
	Viewport* pViewport; //non-owning pointer
	Description* pDescription; //non-owning pointer
	uint32_t width;
	uint32_t height;
	bool viewportIsBound;
	bool descriptionIsBound;
	std::vector<vk::Framebuffer> framebuffers;
	vk::RenderPass renderPass;
	vk::Pipeline graphicsPipeline;
	vk::PipelineLayout pipelineLayout;

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
	void initRasterizer();
	void initMultisampling();
	void initColorAttachment();
	void initColorBlending();
	void initPipelineLayout();
	void initAll();
	void createGraphicsPipeline();
};

#endif /* RENDERER_H */
