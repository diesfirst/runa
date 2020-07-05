#ifndef STATE_VIEWER_HPP
#define STATE_VIEWER_HPP

#include "state.hpp"
#include <command/rendercommands.hpp>
#include <command/commandtypes.hpp>
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
    enum class Op : Option {initialize, loadModel};

    void initialize();

    Xform xform;

    command::pool<command::CompileShader, 2> compileShader;
    command::pool<command::PrepareRenderFrames, 1> prepareRenderFrames;
    command::pool<command::CreateDescriptorSetLayout, 1> createDescriptorSetLayout;
    command::pool<command::CreateFrameDescriptorSets, 1> createFrameDescriptorSets;
    command::pool<command::CreateSwapchainRenderpass, 1> createSwapchainRenderpass;
    command::pool<command::CreatePipelineLayout, 1> createPipelineLayout;
    command::pool<command::CreateGraphicsPipeline, 1> createGraphicsPipeline;
    command::pool<command::CreateRenderLayer, 1> createRenderLayer;
    command::pool<command::AddFrameUniformBuffer, 1> addFrameUniformBuffer;
    command::pool<command::BindUboData, 1> bindUboData;
    command::pool<command::RecordRenderCommand, 1> recordRenderCommand;
    command::pool<command::OpenWindow, 1> openWindow;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_VIEWER_HPP */
