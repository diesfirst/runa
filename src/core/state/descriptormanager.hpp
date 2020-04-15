#ifndef STATE_DESCRIPTORMANAGER_HPP
#define STATE_DESCRIPTORMANAGER_HPP

#include <state/state.hpp>

namespace sword
{

namespace state
{

class DescriptorManager final : public BranchState
{
public:
    enum class Op : Option {};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    const char* getName() const override { return "descriptor_manager"; }
    void handleEvent(event::Event*) override {}
    virtual ~DescriptorManager() = default;
    DescriptorManager(EditStack& es, CommandStack& cs, ExitCallbackFn);
private:
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_DESCRIPTORMANAGER_HPP */
