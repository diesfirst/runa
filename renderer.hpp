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
    int layerId{0};
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

class Attachment
{
public:
	Attachment(
			const vk::Device& device,
			const vk::Extent2D extent);
	Attachment(const vk::Device&, std::unique_ptr<mm::Image>);
	Attachment(const Attachment&) = delete;
	Attachment& operator=(Attachment &&other) = default;
	Attachment& operator=(Attachment&) = delete;
	Attachment(Attachment&& other) = delete;
	virtual ~Attachment();
	vk::Framebuffer& requestFrameBuffer(const RenderPass& renderPass);
	vk::Format getFormat() const;
    vk::Extent2D getExtent() const;
    const mm::Image& getImage(uint32_t index) const;

private:
    friend class RenderFrame;
	const vk::Device& device;
	std::vector<std::unique_ptr<mm::Image>> images;
	vk::Extent2D extent;
	vk::Format format;
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
    Framebuffer(const vk::Device&, const Attachment&, const RenderPass&, const GraphicsPipeline&);
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
    const Attachment& renderTarget;
    const RenderPass& renderPass;
    const GraphicsPipeline& pipeline;
    const vk::Device& device;
};



class RenderFrame
{
public:
	RenderFrame(
			const Context&, 
			std::unique_ptr<Attachment>&& renderTarget, 
			uint32_t width, uint32_t height);
	RenderFrame(const RenderFrame&) = delete;
	RenderFrame& operator=(RenderFrame&) = delete;
	RenderFrame& operator=(RenderFrame&& other) = delete;
	RenderFrame(RenderFrame&& other);
	virtual ~RenderFrame();

	void addOffscreenAttachment(std::unique_ptr<Attachment>&& renderTarget);
	void createDescriptorSets(const std::vector<vk::DescriptorSetLayout>&);
	const std::vector<vk::DescriptorSet>& getDescriptorSets() const;
	vk::Semaphore requestSemaphore();
	CommandBuffer& requestRenderBuffer(uint32_t bufferId); //will reset if exists
    CommandBuffer& getRenderBuffer(uint32_t bufferId);  //will fetch existing
    void addFramebuffer(const Attachment&, const RenderPass&, const GraphicsPipeline&);
    void addFramebuffer(TargetType, const RenderPass&, const GraphicsPipeline&);
    void clearFramebuffers();
	vk::Framebuffer& requestSwapchainFrameBuffer(const RenderPass&);
	vk::Framebuffer& requestOffscreenFrameBuffer(const RenderPass&);
    std::vector<Framebuffer> framebuffers;
    mm::BufferBlock* bufferBlock;

private:
	std::unique_ptr<Attachment> swapchainAttachment;
	std::unique_ptr<Attachment> offscreenAttachment;
	const Context& context;
	const vk::Device& device;
	CommandPool commandPool;
    std::vector<CommandBuffer*> commandBuffers;
//	CommandBuffer* renderBuffer{nullptr};
	vk::DescriptorPool descriptorPool;
	std::vector<vk::DescriptorSet> descriptorSets;
    void updateDescriptorSet(uint32_t setId, const std::vector<vk::WriteDescriptorSet>);
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
	Renderer(const Renderer&) = delete;
	Renderer& operator=(Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	Renderer(Renderer&&) = delete;
	FragShader& loadFragShader(const std::string path, const std::string name);
    VertShader& loadVertShader(const std::string path, const std::string name);
	RenderPass& createRenderPass(std::string name, bool isOffscreen);
    const std::string createDescriptorSetLayout(
            const std::string name, 
            const std::vector<vk::DescriptorSetLayoutBinding>);
    const std::string createPipelineLayout(
            const std::string name, 
            const std::vector<std::string> setLayouts);
    void createFrameDescriptorSets(const std::vector<std::string>setLayoutNames);
    void createOwnDescriptorSets(const std::vector<std::string>setLayoutNames);
    void initFrameUBOs(uint32_t binding);
    void updateFrameSamplers(const vk::ImageView*, const vk::Sampler*, uint32_t binding);
    void updateFrameSamplers(const std::vector<const mm::Image*>, uint32_t binding);
    void prepareAsSwapchainPass(RenderPass&);
    void prepareAsOffscreenPass(RenderPass&);
    Attachment& createAttachment(const std::string name, const vk::Extent2D);
	GraphicsPipeline& createGraphicsPipeline(
			const std::string name, 
            const std::string pipelineLayout,
			const VertShader&,
			const FragShader&,
			const RenderPass&,
			const bool geometric);
	void bindToDescription(Description& description);
	void setFragmentInput(uint32_t index, FragmentInput input);
	//in the future we should disect this function
	//to allow for the renderpasses and 
	//pipeline bindings to happen seperately,
	//having only 1 pipeline per commandbuffer is doomed
	void recordRenderCommands(uint32_t bufferId, std::vector<uint32_t> renderPassIds);
    void prepare(const std::string tarotPath);
    void addFramebuffer(const Attachment&, const RenderPass&, const GraphicsPipeline&);
    void addFramebuffer(TargetType, const RenderPass&, const GraphicsPipeline&);
    void clearFramebuffers();
	void render(uint32_t cmdId);

private:
	const Context& context;
	const vk::Device& device;
	const vk::Queue& graphicsQueue;
	std::vector<RenderFrame> frames;
	std::unique_ptr<Swapchain> swapchain;
	Description* pDescription; //non-owning pointer
	bool descriptionIsBound;
	uint32_t renderPassCount{0};
    std::unordered_map<std::string, std::unique_ptr<Attachment>> attachments;
    Attachment* activeTarget;
    std::vector<FragmentInput> fragUBOs;

    //descriptor stuff
	vk::DescriptorPool descriptorPool;
	std::unique_ptr<mm::Buffer> descriptorBuffer;
	std::vector<vk::DescriptorSet> descriptorSets;
    void updateFramesDescriptorSet(uint32_t setId, const std::vector<vk::WriteDescriptorSet>);
    void updateOwnDescriptorSet(uint32_t setId, const std::vector<vk::WriteDescriptorSet>);

	vk::Semaphore imageAcquiredSemaphore;

	uint32_t activeFrameIndex{0};

	std::unordered_map<std::string, VertShader> vertexShaders;
	std::unordered_map<std::string, FragShader> fragmentShaders;

	std::unordered_map<std::string, vk::DescriptorSetLayout> descriptorSetLayouts;
	std::unordered_map<std::string, vk::PipelineLayout> pipelineLayouts;
	std::unordered_map<std::string, GraphicsPipeline> graphicsPipelines;
	std::unordered_map<std::string, RenderPass> renderPasses;
	void createDefaultDescriptorSetLayout(const std::string name);

	CommandBuffer& beginFrame(uint32_t cmdId);

    void createDescriptorPool();
	void updateFrameDescriptorBuffer(uint32_t frame, const FragmentInput& value);
    void createDescriptorBuffer(uint32_t size);
};

#endif /* RENDERER_H */
