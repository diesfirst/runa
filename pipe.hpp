#ifndef PIPE_H
#define PIPE_H

#include "context.hpp"
#include "geo.hpp"
#include "mem.hpp"

class Pipe
{
public:
	Pipe(const Context&);
	virtual ~Pipe();
	void createGraphicsPipeline(const vk::RenderPass&, const uint32_t width, const uint32_t height);

	void updateDescriptorSets(
			uint32_t descriptorCount, 
			std::vector<bufferBlock>* uboBlocks,
			size_t uboSize);

	vk::Pipeline graphicsPipeline;
	vk::PipelineLayout pipelineLayout;
	std::vector<vk::DescriptorSet> descriptorSets;

	void prepareDescriptors(uint32_t count);

private:
	const Context& context;
	vk::Viewport viewport;
	vk::Rect2D scissor;
	vk::DynamicState dynamicStates; //not used right now
	vk::PipelineColorBlendAttachmentState colorAttachmentState; //could be many
	vk::PipelineColorBlendStateCreateInfo colorBlending;
	vk::PipelineRasterizationStateCreateInfo rasterizer;
	vk::PipelineMultisampleStateCreateInfo multisampling;
	vk::PipelineVertexInputStateCreateInfo vertexInputState;
	vk::PipelineViewportStateCreateInfo viewportState;
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
	vk::DescriptorSetLayout descriptorSetLayout;
	uint32_t descriptorCount;
	bool descriptorsPrepared = false;

	vk::DescriptorPool descriptorPool;
	
	vk::ShaderModule createShaderModule(const std::vector<char>& code);
	void initVertexInputState();
	void initInputAssemblyState();
	void initViewportState();
	void initRasterizer();
	void initMultisampling();
	void initColorAttachment();
	void initColorBlending();
	void initPipelineLayout();


	void initDescriptorSetLayout();
	void createDescriptorPool(uint32_t descriptorCount);
	void createDescriptorSets(uint32_t descriptorCount);
};

#endif /* PIPE_H */
