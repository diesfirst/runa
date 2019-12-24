#ifndef RENDERER_H
#define RENDERER_H

#include "context.hpp"
#include "description.hpp"
#include <memory>
#include <tuple>
#include "window.hpp"
#include "swapchain.hpp"
#include <unordered_map>

enum class ShaderType {vert, frag};

class Shader
{
public:
	virtual ~Shader();
	Shader(const Shader&) = delete; //no copy construction
	Shader(Shader&& other);
	Shader& operator=(Shader& other) = delete; //no copy assignment
	Shader& operator=(Shader&& other) = delete; //no move assignment

	vk::PipelineShaderStageCreateInfo getStageInfo();
	vk::PipelineShaderStageCreateInfo stageInfo;

protected:
	Shader(const vk::Device&, std::string filepath);

private:
	const vk::Device& device;
	std::vector<char> shaderCode;
	size_t codeSize;
	vk::ShaderModule module{nullptr};

	void loadFile(std::string filepath);
	void createModule();
};

class VertShader : public Shader
{
public:
	VertShader(const vk::Device&, std::string);
};

class FragShader : public Shader
{
public:
	FragShader(const vk::Device&, std::string);
};

class RenderPass
{
public:
	RenderPass(const vk::Device&, const vk::Format, uint32_t);
	virtual ~RenderPass();
	RenderPass(const RenderPass&) = delete;
	RenderPass& operator=(RenderPass&) = delete;
	RenderPass& operator=(RenderPass&&) = delete;
	RenderPass(RenderPass&&);
	bool operator<(const RenderPass& rhs);
	const vk::RenderPass& getHandle() const;
	const uint32_t getId() const;
private:
	const vk::Device& device;
	vk::RenderPass handle;
	const uint32_t id;
};

class RenderTarget
{
public:
	RenderTarget(std::unique_ptr<mm::Image>);
	RenderTarget(const RenderTarget&) = delete;
	RenderTarget& operator=(RenderTarget &&other) = default;
	RenderTarget& operator=(RenderTarget&) = delete;
	RenderTarget(RenderTarget&& other) = delete;
	virtual ~RenderTarget();
	std::vector<std::unique_ptr<mm::Image>> images;
private:
};

class RenderFrame
{
public:
	RenderFrame(
			const Context&, 
			std::unique_ptr<RenderTarget>&& renderTarget, 
			uint32_t width, uint32_t height);
	RenderFrame(const RenderFrame&) = delete;
	RenderFrame& operator=(RenderFrame&) = delete;
	RenderFrame& operator=(RenderFrame&& other) = delete;
	RenderFrame(RenderFrame&& other);
	virtual ~RenderFrame();

	void createDescriptorSet(std::vector<vk::DescriptorSetLayout>&);
	vk::Semaphore requestSemaphore();
	CommandBuffer& getRenderBuffer();
	CommandBuffer& requestCommandBuffer();
	vk::Framebuffer& requestFrameBuffer(const RenderPass&);

private:
	std::unique_ptr<RenderTarget> swapchainRenderTarget;
	const Context& context;
	const vk::Device& device;
	CommandPool commandPool;
	CommandBuffer* renderBuffer{nullptr};
	vk::DescriptorPool descriptorPool;
	std::vector<vk::DescriptorSet> descriptorSets;
	vk::Fence fence;
	vk::Semaphore semaphore;
	uint32_t width;
	uint32_t height;
	std::unordered_map<uint32_t, vk::Framebuffer> frameBuffers;

	void createDescriptorPool();
	vk::Framebuffer& createFramebuffer(const RenderPass&);
};

class GraphicsPipeline
{
public:
	GraphicsPipeline(
			const vk::Device&, 
			const vk::PipelineLayout&, 
			const vk::RenderPass&,
			const uint32_t subpassIndex,
			const vk::Rect2D, //we could choose to make this dynamic
			const std::vector<const Shader*>&,
			const vk::PipelineVertexInputStateCreateInfo&);
	~GraphicsPipeline();
	GraphicsPipeline(const GraphicsPipeline&) = delete;
	GraphicsPipeline& operator=(GraphicsPipeline&) = delete;
	GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;
	GraphicsPipeline(GraphicsPipeline&&);

	vk::Viewport createViewport(const vk::Rect2D&);
	vk::Pipeline& getHandle();

	static vk::PipelineVertexInputStateCreateInfo createVertexInputState(
			const std::vector<vk::VertexInputAttributeDescription>&,
			const std::vector<vk::VertexInputBindingDescription>&);

private:
	vk::Pipeline handle;
	const vk::Device& device;

	std::vector<vk::PipelineShaderStageCreateInfo> extractShaderStageInfos(
			const std::vector<const Shader*>& shaders);
	vk::PipelineViewportStateCreateInfo createViewportState(
			const vk::Viewport&,
			const vk::Rect2D&);
	vk::PipelineMultisampleStateCreateInfo createMultisampleState();
	vk::PipelineRasterizationStateCreateInfo createRasterizationState();
	vk::PipelineColorBlendAttachmentState createColorBlendAttachmentState();
	vk::PipelineColorBlendStateCreateInfo createColorBlendState(
			std::vector<vk::PipelineColorBlendAttachmentState>&);
	vk::PipelineDepthStencilStateCreateInfo createDepthStencilState();
	vk::PipelineInputAssemblyStateCreateInfo createInputAssemblyState();
};

class Renderer
{
public:
	Renderer(Context&, XWindow& window);
	~Renderer();
	void createRenderPass(std::string name, vk::Format);
	void createGraphicsPipeline(
			const std::string name, 
			const std::string vertShader,
			const std::string fragShader,
			const std::string renderPassName,
			const bool geometric);
	void bindToDescription(Description& description);
	void update();
	//in the future we should disect this function
	//to allow for the renderpasses and 
	//pipeline bindings to happen seperately,
	//having only 1 pipeline per commandbuffer is doomed
	void recordRenderCommands(const std::string pipelineName, const std::string renderPassName);
	void render();
	void submitDrawCommand(uint32_t index);
	void submitDrawCommand(uint32_t index, vk::Semaphore*);
	void copyImageToSwapImage(Image* image);
	void presentSwapImage(uint32_t index);
	const std::string loadShader(const std::string path, const std::string name, ShaderType); 

private:
	const Context& context;
	const vk::Device& device;
	const vk::Queue& graphicsQueue;
	std::vector<RenderFrame> frames;
	std::unique_ptr<Swapchain> swapchain;
	Description* pDescription; //non-owning pointer
	uint32_t currentFrame;
	bool descriptionIsBound;
	std::vector<vk::Framebuffer> framebuffers;
	std::vector<vk::SubmitInfo> drawSubmitInfos;
	std::vector<vk::PresentInfoKHR> presentInfos;
	uint32_t renderPassCount{0};

	vk::Semaphore imageAcquiredSemaphore;

	uint32_t activeFrameIndex{0};

	std::unordered_map<std::string, VertShader> vertexShaders;
	std::unordered_map<std::string, FragShader> fragmentShaders;

	std::unordered_map<std::string, vk::DescriptorSetLayout> descriptorSetLayouts;
	std::unordered_map<std::string, vk::PipelineLayout> pipelineLayouts;
	std::unordered_map<std::string, std::vector<vk::DescriptorSetLayout>> 
		descriptorSetLayoutSubsets;
	std::unordered_map<std::string, GraphicsPipeline> graphicsPipelines;
	std::unordered_map<std::string, RenderPass> renderPasses;
	//we can have multiple ordered sets of the descriptor set layouts we created.
	//the pipeline layout takes a pointer to an ordered subset of the available 
	//descriptor set layout we have created. if we wanted to make multiple
	//pipelines we could turn this into a set of subsets (a vector or a map) to 
	//create multiple pipeline layouts. not we may be able to avoid this in the
	//pipeline layout creation
	void createDefaultDescriptorSetLayout();
	void createDefaultDescriptorLayoutSubset();
	void createDefaultPipelineLayout();

	CommandBuffer& beginFrame();

	void createDescriptorSets(std::vector<RenderFrame>&, std::string DescriptorSetLayoutKey);

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
};

#endif /* RENDERER_H */
