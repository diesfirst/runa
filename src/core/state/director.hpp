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
    enum class Op : Option {pushRenderManager, printHierarchy};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    const char* getName() const override { return "director"; }
    void handleEvent(event::Event*) override;
    virtual ~Director() = default;
    Director(StateArgs, const StateStack& ss, render::Window& window);

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
