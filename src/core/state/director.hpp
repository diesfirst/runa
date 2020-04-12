#ifndef STATE_DIRECTOR_HPP_
#define STATE_DIRECTOR_HPP_

#include <state/state.hpp>
#include <state/statetypes.hpp>
#include <state/rendermanager.hpp>

namespace sword
{

namespace render { class Window; }

namespace state
{

class Director final : public BranchState
{
public:
    enum class Op : Option {foo, jim, pushRenderManager, printHierarchy};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    const char* getName() const override { return "director"; }
    void handleEvent(event::Event*) override;
    virtual ~Director() = default;
    Director(EditStack& es, CommandStack& cs, const StateStack& ss, render::Window& window) :
        BranchState{es, cs, {
            {"foo", opcast(Op::foo)}, 
            {"jim", opcast(Op::jim)},
            {"render_manager", opcast(Op::pushRenderManager)},
            {"print_state_hierarchy", opcast(Op::printHierarchy)},
        }}, 
        stateStack{ss},
        renderManager{es, cs}
    { 
        activate(opcast(Op::foo));
        activate(opcast(Op::jim));
        activate(opcast(Op::pushRenderManager));
        activate(opcast(Op::printHierarchy));
        keepOn(opcast(Op::printHierarchy));
    }
private:
    void jim();
    void foo();
    void pushRenderManager();
    void printStateHierarchy();
    void popTop();

    RenderManager renderManager;

    const StateStack& stateStack;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: DIRECTOR_HPP_ */
