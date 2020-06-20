#ifndef RENDER_RENDERER_HPP_
#define RENDER_RENDERER_HPP_

#include "vulkan/vulkan.hpp"
#include <memory>
#include <tuple>
#include <unordered_map>
#include <functional>
#include <render/command.hpp>
#include <render/shader.hpp>
#include <render/pipeline.hpp>
#include <render/renderpass.hpp>

namespace sword
{

namespace render
{

enum class ShaderType {vert, frag};

enum class TargetType {offscreen, swapchain};

struct Ubo
{
    void* data;
    uint32_t size;
};

class Window;
class Image;
class BufferBlock;
class RenderFrame;
class Attachment;
class Swapchain;
class Context;
class Buffer;

class Renderer
{
public:
    Renderer(Context&);
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;
    Renderer(Renderer&&) = delete;
    bool loadFragShader(const std::string path, const std::string name);
    bool loadFragShader(std::vector<uint32_t>&& code, const std::string name);
    bool loadVertShader(const std::string path, const std::string name);
    bool loadVertShader(std::vector<uint32_t>&& code, const std::string name);
    RenderPass& createRenderPass(std::string name);
    const std::string createDescriptorSetLayout(
    const std::string name, 
    const std::vector<vk::DescriptorSetLayoutBinding>);
    const std::string createPipelineLayout(
    const std::string name, 
    const std::vector<std::string> setLayouts);
    void prepareRenderFrames(Window& window);
    void createFrameDescriptorSets(const std::vector<std::string>setLayoutNames);
    void createOwnDescriptorSets(const std::vector<std::string>setLayoutNames);
    void initFrameUBOs(size_t size, uint32_t binding);
    void updateFrameSamplers(const vk::ImageView*, const vk::Sampler*, uint32_t binding);
    void updateFrameSamplers(const std::vector<const Image*>&, uint32_t binding);
    void updateFrameSamplers(const std::vector<std::string>& attachmentNames, uint32_t binding);
    void prepareAsSwapchainPass(RenderPass&);
    void prepareAsOffscreenPass(RenderPass&, vk::AttachmentLoadOp);
    Attachment& createAttachment(
    const std::string name, const vk::Extent2D, const vk::ImageUsageFlags);
    bool createGraphicsPipeline(
        const std::string name, 
        const std::string pipelineLayout,
        const std::string vertshader,
        const std::string fragshader,
        const std::string renderpass,
        const vk::Rect2D renderArea,
        const bool geometric);
    bool recreateGraphicsPipeline(
        const std::string name);
    void bindUboData(void* dataPointer, uint32_t size, uint32_t index);
    //in the future we should disect this function
    //to allow for the renderpasses and 
    //pipeline bindings to happen seperately,
    //having only 1 pipeline per commandbuffer is doomed
    void recordRenderCommands(uint32_t bufferId, std::vector<uint32_t> renderPassIds);
    void prepare(const std::string tarotPath);
    void addRenderLayer(
    const std::string attachment, const std::string renderpass,
    const std::string pipeline);
    void clearRenderLayers();
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
    void removeFragmentShader(const std::string name) {fragmentShaders.erase(name);}
    void removeVertexShader(const std::string name) {vertexShaders.erase(name);}
    void removeGraphicsPipeline(const std::string name) {graphicsPipelines.erase(name);}
    void removeRenderpassInstance(int index);

    BufferBlock* copySwapToHost();
    BufferBlock* copyAttachmentToHost(const std::string, const vk::Rect2D region);

    void copyHostToAttachment(void* source, int size, std::string attachmentName, const vk::Rect2D region);

    const Attachment* getAttachmentPtr(std::string name) const;

    vk::Extent2D getSwapExtent();

private:
    const Context& context;
    const vk::Device& device;
    const vk::Queue graphicsQueue;
    std::vector<RenderFrame> frames;
    std::unique_ptr<Swapchain> swapchain;
    bool descriptionIsBound;
    uint32_t renderPassCount{0};
    Attachment* activeTarget;
    std::vector<Ubo> ubos;
    CommandPool commandPool;


    //descriptor stuff
    vk::UniqueDescriptorPool descriptorPool;
    std::unique_ptr<Buffer> hostBuffer;
    std::vector<vk::UniqueDescriptorSet> descriptorSets;
    void updateFramesDescriptorSet(uint32_t setId, const std::vector<vk::WriteDescriptorSet>);
    void updateOwnDescriptorSet(uint32_t setId, const std::vector<vk::WriteDescriptorSet>);

    vk::Semaphore imageAcquiredSemaphore;

    uint32_t activeFrameIndex{0};

    std::unordered_map<std::string, std::unique_ptr<Attachment>> attachments;
    std::unordered_map<std::string, VertShader> vertexShaders;
    std::unordered_map<std::string, FragShader> fragmentShaders;
    std::unordered_map<std::string, vk::UniqueDescriptorSetLayout> descriptorSetLayouts;
    std::unordered_map<std::string, vk::UniquePipelineLayout> pipelineLayouts;
    std::unordered_map<std::string, GraphicsPipeline> graphicsPipelines;
    std::unordered_map<std::string, RenderPass> renderPasses;
    void createDefaultDescriptorSetLayout(const std::string name);

    CommandBuffer& beginFrame(uint32_t cmdId);

    void createDescriptorPool();
    void updateFrameDescriptorBuffer(uint32_t frame, uint32_t uboIndex);
    void createHostBuffer(uint32_t size);
};

}; // namespace render

}; // namespace sword

#endif /* RENDERER_H */
