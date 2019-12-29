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

struct FragmentInput
{
	float time{0};
	float mouseX{0};
	float mouseY{0};	
	bool lmbDown{false};
};

class Shader
{
public:
	virtual ~Shader();
	Shader(const Shader&) = delete; //no copy construction
	Shader(Shader&& other) = delete;
	Shader& operator=(Shader& other) = delete; //no copy assignment
	Shader& operator=(Shader&& other) = delete; //no move assignment

	const vk::PipelineShaderStageCreateInfo& getStageInfo() const;
	void setWindowResolution(const uint32_t w, const uint32_t h);

protected:
	Shader(const vk::Device&, std::string filepath);
	vk::PipelineShaderStageCreateInfo stageInfo;
	std::vector<vk::SpecializationMapEntry> mapEntries;
	vk::SpecializationInfo specInfo;
	std::vector<float> specializationFloats;

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
	RenderPass(const vk::Device&, const std::string name, uint32_t);
	virtual ~RenderPass();
	RenderPass(const RenderPass&) = delete;
	RenderPass& operator=(RenderPass&) = delete;
	RenderPass& operator=(RenderPass&&) = delete;
	RenderPass(RenderPass&&);
	bool operator<(const RenderPass& rhs);
	const vk::RenderPass& getHandle() const;
	uint32_t getId() const;
	void createColorAttachment(
			const vk::Format,
			vk::ImageLayout init,
			vk::ImageLayout fin);
	void createBasicSubpassDependency();
	void createSubpass();
	void create();
private:
	const vk::Device& device;
	const std::string name;
	vk::RenderPass handle;
	std::vector<vk::AttachmentDescription> attachments;
	std::vector<vk::AttachmentDescription> colorAttachments;
	std::vector<vk::AttachmentReference> references;
	std::vector<vk::SubpassDescription> subpasses;
	std::vector<vk::SubpassDependency> subpassDependencies;

	const uint32_t id;
};

class RenderTarget
{
public:
	RenderTarget(
			const vk::Device& device,
			const vk::Extent2D extent);
	RenderTarget(const vk::Device&, std::unique_ptr<mm::Image>);
	RenderTarget(const RenderTarget&) = delete;
	RenderTarget& operator=(RenderTarget &&other) = default;
	RenderTarget& operator=(RenderTarget&) = delete;
	RenderTarget(RenderTarget&& other) = delete;
	virtual ~RenderTarget();
	std::vector<std::unique_ptr<mm::Image>> images;
	vk::Framebuffer& requestFrameBuffer(const RenderPass& renderPass);
private:
	const vk::Device& device;
	vk::Extent2D extent;
	std::unordered_map<uint32_t, vk::Framebuffer> frameBuffers;
	vk::Framebuffer& createFramebuffer(const RenderPass& renderPass);
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
	void createDescriptorBuffer(bool map);
	const std::vector<vk::DescriptorSet>& getDescriptorSets() const;
	mm::Buffer& getDescriptorBuffer();
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
	std::unique_ptr<mm::Buffer> descriptorBuffer;
	std::vector<vk::DescriptorSet> descriptorSets;
	vk::Fence fence;
	vk::Semaphore semaphore;
	uint32_t width;
	uint32_t height;

	void createDescriptorPool();
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
	vk::PipelineLayout& getLayout();

	static vk::PipelineVertexInputStateCreateInfo createVertexInputState(
			const std::vector<vk::VertexInputAttributeDescription>&,
			const std::vector<vk::VertexInputBindingDescription>&);

private:
	vk::Pipeline handle;
	const vk::Device& device;
	vk::PipelineLayout layout;

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
	void setFragmentInput(FragmentInput input);
	//in the future we should disect this function
	//to allow for the renderpasses and 
	//pipeline bindings to happen seperately,
	//having only 1 pipeline per commandbuffer is doomed
	void recordRenderCommands(
			const std::string pipelineName, 
			const std::string renderPassName);
	void render();
	const std::string loadShader(const std::string path, const std::string name, ShaderType); 

private:
	const Context& context;
	const vk::Device& device;
	const vk::Queue& graphicsQueue;
	std::vector<RenderFrame> frames;
	std::unique_ptr<Swapchain> swapchain;
	Description* pDescription; //non-owning pointer
	bool descriptionIsBound;
	uint32_t renderPassCount{0};

	FragmentInput fragmentInput;

	vk::Semaphore imageAcquiredSemaphore;

	uint32_t activeFrameIndex{0};

	std::unordered_map<std::string, VertShader> vertexShaders;
	std::unordered_map<std::string, FragShader> fragmentShaders;

	std::unordered_map<std::string, vk::DescriptorSetLayout> descriptorSetLayouts;
	std::unordered_map<std::string, vk::PipelineLayout> pipelineLayouts;
	std::unordered_map<std::string, GraphicsPipeline> graphicsPipelines;
	std::unordered_map<std::string, RenderPass> renderPasses;
	void createDefaultDescriptorSetLayout(const std::string name);
	void createPipelineLayout(
			const std::string name, 
			const std::vector<std::string> descriptorSetLayoutNames);

	CommandBuffer& beginFrame();

	void updateDescriptor(uint32_t frame, const FragmentInput& value);

	void createDescriptorSets(
			std::vector<RenderFrame>&, 
			const std::vector<std::string> setLayoutNames);
};

#endif /* RENDERER_H */
