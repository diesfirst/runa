#ifndef RENDERER_H
#define RENDERER_H

#include "context.hpp"
#include "description.hpp"
#include <memory>
#include <tuple>
#include "window.hpp"
#include "swapchain.hpp"
#include <unordered_map>
#include <functional>

enum class ShaderType {vert, frag};

enum class TargetType {offscreen, swapchain};

struct FragmentInput
{
	float time{0};
	float mouseX{0};
	float mouseY{0};	
	int blur{0};
    float r{1.};
    float g{1.};
    float b{1.};
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
	RenderPass(const vk::Device&, const std::string name, bool isOffscreen, uint32_t id);
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
			vk::ImageLayout fin,
            vk::ClearColorValue,
            vk::AttachmentLoadOp);
	void addSubpassDependency(vk::SubpassDependency);
	void createBasicSubpassDependency();
	void createSubpass();
	void create();
	bool isCreated() const;
    bool isOffscreen() const;
    const vk::ClearValue* getClearValue() const;

private:
	const vk::Device& device;
	const std::string name;
	vk::RenderPass handle{nullptr};
	std::vector<vk::AttachmentDescription> attachments;
	std::vector<vk::AttachmentDescription> colorAttachments;
	std::vector<vk::AttachmentReference> references;
	std::vector<vk::SubpassDescription> subpasses;
	std::vector<vk::SubpassDependency> subpassDependencies;
    vk::ClearValue clearValue;
    const bool offscreen;

	const uint32_t id;
	bool created{false};
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
	vk::Format getFormat() const;
    vk::Extent2D getExtent() const;
private:
    friend class RenderFrame;
	const vk::Device& device;
	vk::Extent2D extent;
	vk::Format format;
	std::unordered_map<uint32_t, vk::Framebuffer> frameBuffers;
	vk::Framebuffer& createFramebuffer(const RenderPass& renderPass);
};

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
			const std::vector<const Shader*>&,
			const vk::PipelineVertexInputStateCreateInfo&);
	~GraphicsPipeline();
	GraphicsPipeline(const GraphicsPipeline&) = delete;
	GraphicsPipeline& operator=(GraphicsPipeline&) = delete;
	GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;
	GraphicsPipeline(GraphicsPipeline&&);

	vk::Viewport createViewport(const vk::Rect2D&);
	const vk::Pipeline& getHandle() const;
	const vk::PipelineLayout& getLayout() const;
	const RenderPass& getRenderPass() const;
    vk::Rect2D getRenderArea() const;

	static vk::PipelineVertexInputStateCreateInfo createVertexInputState(
			const std::vector<vk::VertexInputAttributeDescription>&,
			const std::vector<vk::VertexInputBindingDescription>&);

private:
    const std::string name;
	vk::Pipeline handle;
	const vk::Device& device;
	const RenderPass& renderPass;
	vk::PipelineLayout layout;
    vk::Rect2D renderArea;

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

class Framebuffer
{
public:
    Framebuffer(const vk::Device&, const RenderTarget&, const RenderPass&, const GraphicsPipeline&);
    virtual ~Framebuffer();
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(Framebuffer&) = delete;
	Framebuffer& operator=(Framebuffer&&) = delete;
	Framebuffer(Framebuffer&&);
    const RenderPass& getRenderPass() const;
    const GraphicsPipeline& getPipeline() const;
    const vk::Framebuffer& getHandle() const;
private:
    vk::Framebuffer handle;
    const RenderTarget& renderTarget;
    const RenderPass& renderPass;
    const GraphicsPipeline& pipeline;
    const vk::Device& device;
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

	void addOffscreenRenderTarget(std::unique_ptr<RenderTarget>&& renderTarget);
	void createDescriptorSet(std::vector<vk::DescriptorSetLayout>&, const mm::Image&);
	void createDescriptorBuffer(bool map);
	const std::vector<vk::DescriptorSet>& getDescriptorSets() const;
	mm::Buffer& getDescriptorBuffer();
	vk::Semaphore requestSemaphore();
	CommandBuffer& getRenderBuffer();
	CommandBuffer& requestCommandBuffer();
    void addFramebuffer(const RenderTarget&, const RenderPass&, const GraphicsPipeline&);
    void addFramebuffer(TargetType, const RenderPass&, const GraphicsPipeline&);
	vk::Framebuffer& requestSwapchainFrameBuffer(const RenderPass&);
	vk::Framebuffer& requestOffscreenFrameBuffer(const RenderPass&);
    std::vector<Framebuffer> framebuffers;

private:
	std::unique_ptr<RenderTarget> swapchainRenderTarget;
	std::unique_ptr<RenderTarget> offscreenRenderTarget;
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

class Renderer
{
public:
	Renderer(Context&, XWindow& window);
	~Renderer();
	FragShader& loadFragShader(const std::string path, const std::string name);
    VertShader& loadVertShader(const std::string path, const std::string name);
	RenderPass& createRenderPass(std::string name, bool isOffscreen);
    void prepareAsSwapchainPass(RenderPass&);
    void prepareAsOffscreenPass(RenderPass&);
	GraphicsPipeline& createGraphicsPipeline(
			const std::string name, 
			const VertShader&,
			const FragShader&,
			const RenderPass&,
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
    void recordRenderCommandsTest();
	void recordRenderCommandsTest2(std::vector<const GraphicsPipeline*>);
	void recordRenderCommandsTest3();
    void prepare(const std::string tarotPath);
    void prepare2(const std::string tarotPath);
	void render();

private:
	const Context& context;
	const vk::Device& device;
	const vk::Queue& graphicsQueue;
	std::vector<RenderFrame> frames;
	std::unique_ptr<Swapchain> swapchain;
	Description* pDescription; //non-owning pointer
	bool descriptionIsBound;
	uint32_t renderPassCount{0};
    std::unique_ptr<RenderTarget> paint;

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
