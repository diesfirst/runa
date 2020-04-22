#ifndef STATE_RENDERPASSMANAGER_HPP
#define STATE_RENDERPASSMANAGER_HPP

//imp: state/renderpassmanager.cpp

#include <state/state.hpp>
#include <command/rendercommands.hpp>

namespace sword
{

namespace state
{

enum class RenderPassType : uint8_t {swapchain, offscreen};

class CreateRenderPass : public LeafState
{
public:
    const char* getName() const override { return "CreateRenderPass"; }
    void handleEvent(event::Event*) override;
    virtual ~CreateRenderPass() = default;   
    CreateRenderPass(StateArgs, Callbacks);
    void setType(RenderPassType);
private:
    void onEnterExt() override;
    RenderPassType type;
    CommandPool<command::CreateSwapchainRenderpass>& csrpPool;
    CommandPool<command::CreateOffscreenRenderpass>& corpPool;
};

class RenderPassManager final : public BranchState
{
public:
    const char* getName() const override { return "rpass_manager"; }
    void handleEvent(event::Event*) override;
    virtual ~RenderPassManager() = default;
    RenderPassManager(StateArgs sa, Callbacks cb, ReportCallbackFn<RenderPassReport>);
    void activateCreateSwap();
private:
    enum class Op : Option {createSwapRpass, createOffscreenRpass, printReports};
    CreateRenderPass createRenderPass;
    void pushCreateRenderPass(RenderPassType);
    void printReports();

    Reports<RenderPassReport> reports;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_RENDERPASSMANAGER_HPP */
