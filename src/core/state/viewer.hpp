//imp: viewer.cpp
#ifndef STATE_VIEWER_HPP
#define STATE_VIEWER_HPP

#include "state.hpp"
#include <command/rendercommands.hpp>
#include <command/commandtypes.hpp>
#include <render/resource.hpp>
#include <glm/glm.hpp>

namespace sword
{

namespace state
{

struct Xform
{
    glm::mat4 view;;
};

class Viewer final : public BranchState
{
public:
    const char* getName() const override { return "Viewer"; }
    void handleEvent(event::Event*) override;
    virtual ~Viewer() = default;
    Viewer(StateArgs, Callbacks);
private:
    enum class Op : Option {initialize, loadModel, initialize2};

    void initialize();
    void initialize2();

    Xform xform;

    render::BufferBlock* stagingVertBuffer{nullptr};
    render::BufferBlock* deviceVertBuffer{nullptr};

    command::Pool<command::CompileShader, 2> compileShader;
    command::Pool<command::PrepareRenderFrames, 1> prepareRenderFrames;
    command::Pool<command::CreateDescriptorSetLayout, 1> createDescriptorSetLayout;
    command::Pool<command::CreateFrameDescriptorSets, 1> createFrameDescriptorSets;
    command::Pool<command::CreateSwapchainRenderpass, 1> createSwapchainRenderpass;
    command::Pool<command::CreatePipelineLayout, 1> createPipelineLayout;
    command::Pool<command::CreateGraphicsPipeline, 1> createGraphicsPipeline;
    command::Pool<command::CreateRenderLayer, 1> createRenderLayer;
    command::Pool<command::AddFrameUniformBuffer, 1> addFrameUniformBuffer;
    command::Pool<command::BindUboData, 1> bindUboData;
    command::Pool<command::RecordRenderCommand, 1> recordRenderCommand;
    command::Pool<command::OpenWindow, 1> openWindow;
    command::Pool<command::RequestBufferBlock, 2> requestBufferBlock;
    command::Pool<command::SetRenderCommand, 2> setRenderCommand;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_VIEWER_HPP */
