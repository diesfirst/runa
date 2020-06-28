#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

#include <event/event.hpp>
#include <state/director.hpp>
#include <state/statetypes.hpp>
#include <state/editstack.hpp>
#include <render/ubotypes.hpp>
#include <render/context.hpp>
#include <render/surface/window.hpp>
#include <render/renderer.hpp>
#include <string>
#include <command/commandpools.hpp>
#include <event/dispatcher.hpp>

namespace sword
{

class Application
{
public:
    Application(bool validate = true);
    Application(uint16_t w, uint16_t h, const std::string logfile, int eventPops = 0);
    void run(bool pollEvents);
    void popState();
    void pushState(state::State* const);
    void pushCmd(CmdPtr&&);

    void readEvents(std::ifstream&, int eventPops);
    void recordEvent(event::Event* event, std::ofstream& os);

    render::Context context;
    render::Window window;
    render::Renderer renderer;
    event::EventDispatcher dispatcher;
    vk::Extent2D offscreenDim;
    vk::Extent2D swapDim;;
    std::ofstream os;
    std::ifstream is;

    CommandPools cmdPools;
    state::Register stateRegister;
   
    std::vector<const render::Image*> sampledImages;

private:
    std::string readlog;
    std::string writelog{"eventlog"};
    bool recordevents{true};
    bool readevents{false};
    
    state::EditStack stateEdits;
    StateStack stateStack;
    CommandStack cmdStack;

    state::Director dirState;

    void beginFrame();
    void endFrame();
    void drainEventQueue();
    void executeCommands();

    int maxEventReads{0};
    int eventsRead{0};

    event::EventPool<event::BeginFrame, 2> beginFramePool;
    event::EventPool<event::EndFrame, 2> endFramePool;
};

}; // namespace sword

#endif /* end of include guard: APPLICATION_HPP_ */
