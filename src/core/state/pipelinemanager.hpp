#ifndef STATE_PIPELINEMANAGER_HPP
#define STATE_PIPELINEMANAGER_HPP

#include <state/state.hpp>

namespace sword
{

namespace state
{

class PipelineManager final : public BranchState
{
public:
    enum class Op : Option {};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    const char* getName() const override { return "pipeline_manager"; }
    void handleEvent(event::Event*) override {}
    virtual ~PipelineManager () = default;
    PipelineManager (StateArgs sa)  : 
        BranchState{sa,{
            {},
        }}
    {   
    }
private:
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_PIPELINEMANAGER_HPP */
