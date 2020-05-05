#ifndef STATE_DIRECTOR_HPP_
#define STATE_DIRECTOR_HPP_

#include <state/state.hpp>
#include <state/statetypes.hpp>
#include <state/rendermanager.hpp>

//imp: state/director.cpp

namespace sword
{

namespace render { class Window; }

namespace state
{

class QuickSetup : public BriefState
{
public:
    const char* getName() const override { return "QuickSetup"; }
    QuickSetup(StateArgs, Callbacks);
private:
    void onEnterExt() override;

    state::Register& sr;
    CommandPools& cp;
};

class Director final : public BranchState
{
public:
    const char* getName() const override { return "director"; }
    void handleEvent(event::Event*) override;
    Director(StateArgs, const StateStack& ss, render::Window& window);

private:
    enum class Op : Option {pushRenderManager, printHierarchy, quickSetup, quickSetup2, quickSetup3};

    void pushRenderManager();
    void printStateHierarchy();
    void popTop();
    void quickSetup();
    void quickSetup2();
    void quickSetup3();

    RenderManager renderManager;
    QuickSetup quickState;

    CommandPools& cp;
    state::Register& sr;

    const StateStack& stateStack;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: DIRECTOR_HPP_ */
