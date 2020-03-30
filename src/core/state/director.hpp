#ifndef STATE_DIRECTOR_HPP_
#define STATE_DIRECTOR_HPP_

#include <state/state.hpp>

namespace sword
{

namespace state
{

class Director : public BranchState
{
public:
    enum class Op : Option {foo, jim};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    Director(EditStack& es, CommandStack& cs) :
        BranchState{es, cs, {
            {"foo", opcast(Op::foo)}, 
            {"jim", opcast(Op::jim)}
        }} 
    {
        opmask.set(opcast(Op::foo));
        opmask.set(opcast(Op::jim));
    }
    const char* getName() const override { return "director"; }
    void handleEvent(event::Event*) override;
    void dynamicOpsActive(bool b) override {}
    void jim();
    void foo();
private:
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: DIRECTOR_HPP_ */
