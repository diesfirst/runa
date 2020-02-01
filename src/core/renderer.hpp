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

struct SpecData
{
    float windowWidth{256};
    float windowHeight{256};
    int integer0{0};
    int integer1{0};
};

struct Ubo
{
    void* data;
    uint32_t size;
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

    SpecData specData;

protected:
	Shader(const vk::Device&, std::string filepath);
	vk::PipelineShaderStageCreateInfo stageInfo;
	std::vector<vk::SpecializationMapEntry> mapEntries;
	vk::SpecializationInfo specInfo;

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
	RenderPass(const vk::Device&, const std::string name);
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

	bool created{false};
};

class Attachment
{
public:
	Attachment(
			const vk::Device& device,
			const vk::Extent2D extent,
            const vk::ImageUsageFlags);
	Attachment(const vk::Device&, std::unique_ptr<mm::Image>);
	Attachment(const Attachment&) = delete;
	Attachment& operator=(Attachment &&other) = default;
	Attachment& operator=(Attachment&) = delete;
	Attachment(Attachment&& other) = delete;
	virtual ~Attachment();
	vk::Framebuffer& requestFrameBuffer(const RenderPass& renderPass);
	vk::Format getFormat() const;
    vk::Extent2D getExtent() const;
    mm::Image& getImage(uint32_t index);

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
			const std::vector<const Shader*>,
			const vk::PipelineVertexInputStateCreateInfo);
	~GraphicsPipeline();
	GraphicsPipeline(const GraphicsPipeline&) = delete;
	GraphicsPipeline& operator=(GraphicsPipeline&) = delete;
	GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;
	GraphicsPipeline(GraphicsPipeline&&);

    void create();
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
	vk::Pipeline handle;
	const vk::Device& device;
	const RenderPass& renderPass;
	vk::PipelineLayout layout;
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

class RenderPassInstance
{
public:
    RenderPassInstance(const vk::Device&, Attachment&, const RenderPass&, const GraphicsPipeline&);
    virtual ~RenderPassInstance();
	RenderPassInstance(const RenderPassInstance&) = delete;
	RenderPassInstance& operator=(RenderPassInstance&) = delete;
	RenderPassInstance& operator=(RenderPassInstance&&) = delete;
	RenderPassInstance(RenderPassInstance&&);
    const RenderPass& getRenderPass() const;
    const GraphicsPipeline& getPipeline() const;
    const vk::Framebuffer& getFramebuffer() const;
private:
    vk::Framebuffer framebuffer;
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

    Attachment& getSwapAttachment();
	void addOffscreenAttachment(std::unique_ptr<Attachment>&& renderTarget);
	void createDescriptorSets(const std::vector<vk::DescriptorSetLayout>&);
	const std::vector<vk::DescriptorSet>& getDescriptorSets() const;
	vk::Semaphore requestSemaphore();
	CommandBuffer& requestRenderBuffer(uint32_t bufferId); //will reset if exists
    CommandBuffer& getRenderBuffer(uint32_t bufferId);  //will fetch existing
    void addRenderPassInstance(Attachment&, const RenderPass&, const GraphicsPipeline&);
    void addRenderPassInstance(const RenderPass&, const GraphicsPipeline&);
    void clearRenderPassInstances();
    std::vector<RenderPassInstance> renderPassInstances;
    mm::BufferBlock* bufferBlock;
    

private:
	std::unique_ptr<Attachment> swapchainAttachment;
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
	Renderer(Context&);
	~Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	Renderer(Renderer&&) = delete;
	FragShader& loadFragShader(const std::string path, const std::string name);
    VertShader& loadVertShader(const std::string path, const std::string name);
	RenderPass& createRenderPass(std::string name);
    const std::string createDescriptorSetLayout(
            const std::string name, 
            const std::vector<vk::DescriptorSetLayoutBinding>);
    const std::string createPipelineLayout(
            const std::string name, 
            const std::vector<std::string> setLayouts);
    void prepareRenderFrames(XWindow& window);
    void createFrameDescriptorSets(const std::vector<std::string>setLayoutNames);
    void createOwnDescriptorSets(const std::vector<std::string>setLayoutNames);
    void initFrameUBOs(size_t size, uint32_t binding);
    void updateFrameSamplers(const vk::ImageView*, const vk::Sampler*, uint32_t binding);
    void updateFrameSamplers(const std::vector<const mm::Image*>&, uint32_t binding);
    void prepareAsSwapchainPass(RenderPass&);
    void prepareAsOffscreenPass(RenderPass&, vk::AttachmentLoadOp);
    Attachment& createAttachment(
            const std::string name, const vk::Extent2D, const vk::ImageUsageFlags);
	GraphicsPipeline& createGraphicsPipeline(
			const std::string name, 
            const std::string pipelineLayout,
			const std::string vertshader,
			const std::string fragshader,
			const std::string renderpass,
            const vk::Rect2D renderArea,
			const bool geometric);
	void bindToDescription(Description& description);
	void bindUboData(void* dataPointer, uint32_t size, uint32_t index);
	//in the future we should disect this function
	//to allow for the renderpasses and 
	//pipeline bindings to happen seperately,
	//having only 1 pipeline per commandbuffer is doomed
	void recordRenderCommands(uint32_t bufferId, std::vector<uint32_t> renderPassIds);
    void prepare(const std::string tarotPath);
    void addRenderPassInstance(
            const std::string attachment, const std::string renderpass,
            const std::string pipeline);
    void clearRenderPassInstances();
	void render(uint32_t cmdId, bool updateUbo);
    void popBufferBlock();
    void listAttachments() const;
    void listVertShaders() const;
    void listFragShaders() const;
    void listPipelineLayouts() const;
    void listRenderPasses() const;
    FragShader& fragShaderAt(const std::string);
    VertShader& vertShaderAt(const std::string);
    RenderPass& renderPassAt(const std::string);

    mm::BufferBlock* copySwapToHost(const vk::Rect2D region);
    mm::BufferBlock* copyAttachmentToHost(const std::string, const vk::Rect2D region);

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
    std::vector<Ubo> ubos;
    CommandPool commandPool;


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
	void updateFrameDescriptorBuffer(uint32_t frame, uint32_t uboIndex);
    void createDescriptorBuffer(uint32_t size);
};

#endif /* RENDERER_H */
