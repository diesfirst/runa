#ifndef STATE_RENDERMANAGER_HPP
#define STATE_RENDERMANAGER_HPP

#include <state/state.hpp>

namespace sword
{

namespace render{ class Window; };

namespace state
{

class RenderManager final : public BranchState
{
public:
    enum class Op : Option {openWindow};
    constexpr Option opcast(Op op) {return static_cast<Option>(op);}
    constexpr Op opcast(Option op) {return static_cast<Op>(op);}
    const char* getName() const override { return "render_manager"; }
    void handleEvent(event::Event*) override;
    virtual ~RenderManager() = default;
    RenderManager(EditStack& es, CommandStack& cs, render::Window& w)  : 
        BranchState{es, cs,{
            {"open_window", opcast(Op::openWindow)},
        }},
        window{w}
    {   
        activate(opcast(Op::openWindow));
    }
private:
    void openWindow();
    render::Window& window;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: STATE_RENDERMANAGER_HPP */
