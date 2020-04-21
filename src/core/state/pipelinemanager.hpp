#ifndef STATE_PIPELINEMANAGER_HPP
#define STATE_PIPELINEMANAGER_HPP

#include <state/state.hpp>

namespace sword
{

namespace state
{

class CreateGraphicsPipeline : public LeafState
{
public:
    const char* getName() const override { return "CreateGraphicsPipeline"; }
    void handleEvent(event::Event*) override;
    virtual ~CreateGraphicsPipeline() = default;   
    CreateGraphicsPipeline(StateArgs, Callbacks);
private:
    void onEnterExt() override;
};

class CreatePipelineLayout final : public LeafState
{
public:
    const char* getName() const override { return "CreatePipelineLayout final"; }
    void handleEvent(event::Event*) override;
    virtual ~CreatePipelineLayout() = default;   
    CreatePipelineLayout(StateArgs, Callbacks);
private:
    void onEnterExt() override;
};

class PipelineManager final : public BranchState
{
public:
    const char* getName() const override { return "pipeline_manager"; }
    void handleEvent(event::Event*) override;
    virtual ~PipelineManager () = default;
    PipelineManager (StateArgs, Callbacks);
private:
    enum class Op : Option {createPipelineLayout, createGraphicsPipeline};

    CreatePipelineLayout createPipelineLayout;
    CreateGraphicsPipeline createGraphicsPipeline;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_PIPELINEMANAGER_HPP */
