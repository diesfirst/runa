#ifndef STATE_RENDERPASSMANAGER_HPP
#define STATE_RENDERPASSMANAGER_HPP

#include <state/state.hpp>

namespace sword
{

namespace state
{

class RenderpassManager final : public BranchState
{
public:
    enum class Op : Option {};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    const char* getName() const override { return "rpass_manager"; }
    void handleEvent(event::Event*) override {}
    virtual ~RenderpassManager() = default;
    RenderpassManager(EditStack& es, CommandStack& cs)  : 
        BranchState{es, cs,{
            {},
        }}
    {   
    }
private:
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_RENDERPASSMANAGER_HPP */
