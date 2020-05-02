//imp: state/pipelinemanager.cpp
#ifndef STATE_PIPELINEMANAGER_HPP
#define STATE_PIPELINEMANAGER_HPP


#include <state/state.hpp>
#include <command/rendercommands.hpp>

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
    CommandPool<command::CreateGraphicsPipeline>& pool;
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
    CommandPool<command::CreatePipelineLayout>& pool;

    void onEnterExt() override;
};

class PipelineManager final : public BranchState
{
public:
    const char* getName() const override { return "pipeline_manager"; }
    void handleEvent(event::Event*) override;
    virtual ~PipelineManager () = default;
    PipelineManager (StateArgs, Callbacks);
    void receiveReport(const Report*);
private:
    enum class Op : Option {createPipelineLayout, createGraphicsPipeline, printReports};

    CreatePipelineLayout createPipelineLayout;
    CreateGraphicsPipeline createGraphicsPipeline;

    Reports<PipelineLayoutReport> pipeLayoutReports;
    Reports<GraphicsPipelineReport> graphicsPipeReports;

    std::vector<const Report*> reports;

    CommandPool<command::CreateGraphicsPipeline>& cgpPool;

    void printReports();
    void handleShaderReport(const ShaderReport* pReport);
    void recreateGraphicsPipeline(GraphicsPipelineReport*);
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_PIPELINEMANAGER_HPP */
