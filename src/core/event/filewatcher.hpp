#ifndef EVENT_FILEWATCHER_HPP
#define EVENT_FILEWATCHER_HPP

#include <string>
#include "types.hpp"
#include "event.hpp"
#include <types/map.hpp>

namespace sword
{

namespace event
{

struct WatchTicket
{
    int wd;
    const char* fullpath;
    std::string filename;
};

class FileWatcher
{
public:
    FileWatcher(EventQueue& queue);
    void run();
    void stop();
    bool addWatch(const char* path); //all watchers will be modify for now

private:
    static constexpr int name_max_len{30}; //no shader filename can be longer than this
    int fd;
    bool shouldStop{false};
    EventQueue& eventQueue;
    std::vector<WatchTicket> watches;

    Pool<File, 3> eventPool;
};

}; // namespace event

}; // namespace sword

#endif /* end of include guard: EVENT_FILEWATCHER_HPP */
