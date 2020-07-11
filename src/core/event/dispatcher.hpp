#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include "event.hpp"
#include "types.hpp"
#include "filewatcher.hpp"
#include <mutex>

namespace sword
{

namespace event
{

//this class is not thread safe at all
class EventDispatcher
{
public:
    EventDispatcher(const render::Window&);
    ~EventDispatcher();

    void setVocabulary(std::vector<std::string> vocab);
    static void updateVocab();
    void addVocab(const state::Vocab*);
    void popVocab();

    void fetchWindowInput();
    void fetchCommandLineInput();
    void pollEvents();

    void runCommandLineLoop();
    void runWindowInputLoop();
    void readEvents(std::ifstream& is, int eventPops);
    void readEvent(std::ifstream& is);

    void getNextEvent();

    WindowInput windowInput;
    std::string commandLineInput;
    inline void setInputMode(InputMode mode) {inputMode = mode;}

    EventQueue eventQueue;
    FileWatcher fileWatcher;

    std::mutex lock;

private:
    const render::Window& window;
    InputMode inputMode{InputMode::CommandLine};

    inline static std::vector<std::string> vocabulary{};
    inline static std::vector<const state::Vocab*> vocabPtrs;
    static char* completion_generator(const char* text, int state);
    static char** completer(const char* text, int start, int end);
    bool keepWindowThread{true};
    bool keepCommandThread{true};

    //pools
    Pool<CommandLine, 200> clPool;
    Pool<KeyPress, 50> kpPool;
    Pool<KeyRelease, 20> krPool;
    Pool<MousePress, 20> mpPool;
    Pool<MouseRelease, 20> mrPool;
    Pool<MouseMotion, 100> mmPool;
    Pool<Abort, 50> aPool;
    Pool<LeaveWindow, 3> lwPool;
};

}; // namespace event

}; // namespace sword

#endif /* end of include guard: DISPATCHER_HPP */
