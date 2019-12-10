#ifndef RENDERER_H
#define RENDERER_H

#include "context.hpp"
#include "description.hpp"
#include <memory>
#include <tuple>

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
	void bindToDescription(Description& description);
	void setup(Viewport&, Description&);
	void update();
	void render();
	void submitDrawCommand(uint32_t index);
	void submitDrawCommand(uint32_t index, vk::Semaphore*);
	void copyImageToSwapImage(Image* image);
	void presentSwapImage(uint32_t index);

private:
	const Context& context;
	const vk::Device& device;
	const vk::Queue& graphicsQueue;
	Viewport* pViewport; //non-owning pointer
	Description* pDescription; //non-owning pointer
	uint32_t width;
	uint32_t height;
	uint32_t currentFrame;
	bool viewportIsBound;
	bool descriptionIsBound;
	std::vector<vk::Framebuffer> framebuffers;
	vk::RenderPass renderPass;
	vk::Pipeline graphicsPipeline;
	vk::PipelineLayout pipelineLayout;
	std::unique_ptr<CommandBlock> drawCommandBlock;
	std::vector<vk::SubmitInfo> drawSubmitInfos;
	std::vector<vk::PresentInfoKHR> presentInfos;

	vk::DynamicState dynamicStates; //not used right now
	vk::PipelineColorBlendAttachmentState colorAttachmentState; //could be many
	vk::PipelineColorBlendStateCreateInfo colorBlending;
	vk::PipelineRasterizationStateCreateInfo rasterizer;
	vk::PipelineMultisampleStateCreateInfo multisampling;
	vk::PipelineVertexInputStateCreateInfo vertexInputState;
	vk::PipelineViewportStateCreateInfo viewportState;
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;

	//these will be passed to the swap image acquisition function
	//and then to the draw function
	//by passing to the image acquisition function we say we want it 
	//to be signalled when we can write to that image
	//and the draw function will wait to write to the image until it is signalled
	//
	//in the future, there may be multiple swap chains we want to render to
	//so this will likely become part of a struct of both swap chains and semaphores
	//and associated frame counter
	//void initSwapImageSemaphores(uint32_t count);
	void initDrawSubmitInfos(uint32_t count);
	void initPresentInfos(uint32_t count);
	uint8_t maxFramesInFlight;
	
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
