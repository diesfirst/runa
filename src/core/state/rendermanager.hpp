#ifndef STATE_RENDERMANAGER_HPP
#define STATE_RENDERMANAGER_HPP

//imp : state/rendermanager.cpp

#include <state/state.hpp>
#include <state/pipelinemanager.hpp>
#include <state/descriptormanager.hpp>
#include <state/renderpassmanager.hpp>
#include <state/shader.hpp>
#include <command/rendercommands.hpp>

namespace sword
{

namespace state
{

class OpenWindow : public BriefState
{
public:
    const char* getName() const override { return "OpenWindow"; }
    OpenWindow(StateArgs, Callbacks);
private:
    void onEnterExt() override;
    CommandPool<command::OpenWindow>& owPool;
};

class PrepareRenderFrames : public BriefState
{
public:
    const char* getName() const override { return "PrepareRenderFrames"; }
    PrepareRenderFrames(StateArgs, Callbacks);
private:
    void onEnterExt() override;
    CommandPool<command::PrepareRenderFrames>& prfPool;
};

class RenderManager final : public BranchState
{
public:
    const char* getName() const override { return "render_manager"; }
    void handleEvent(event::Event*) override;
    RenderManager(StateArgs, Callbacks cb);
private:
    enum class Op : Option {openWindow, prepRenderFrames, shaderManager, descriptorManager, renderPassManager, pipelineManager};

    PipelineManager pipelineManager;
    RenderPassManager rpassManager;
    DescriptorManager descriptorManager;
    ShaderManager shaderManager;
    OpenWindow openWindow;
    PrepareRenderFrames prepRenderFrames;

    bool createdDescSetLayout{false};

    void onPrepRenderFrames();
    void onOpenWindow();
    void receiveDescriptorSetLayoutReport(const DescriptorSetLayoutReport*);
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_RENDERMANAGER_HPP */
