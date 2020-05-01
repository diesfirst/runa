#include "filewatcher.hpp"
#include <unistd.h>
#include <iostream>
#include <util/outformat.hpp>
#include <sys/inotify.h>
#include <filesystem>

namespace sword
{

namespace event
{

FileWatcher::FileWatcher(EventQueue& queue) :
    eventQueue{queue}
{
    curName.reserve(name_max_len);
}

// due to the way vim operates, we cannot watch the file itself.
// vim actually makes a new file on save and then deletes the old one.
// this is actually a good thing as it ensures we don't ever try to to compile 
// the file mid-write. instead, we watch the directory, and then only generate
// an event when the read buffer contains the name of the file we actually want 
// to watch
bool FileWatcher::addWatch(const char* path_str)
{
    std::filesystem::path path{path_str};
    auto parent = path.parent_path();
    int watch = inotify_add_watch(fd, parent.c_str(), IN_CLOSE_WRITE);
    if (watch == -1)
        return false;
    std::string name = path.stem();
    name.append(path.extension());
    watches.push(std::pair(watch, name));
    std::cout << "watching " << name << '\n';
    return true;
}

void FileWatcher::run()
{
    fd = inotify_init();
    SWD_THREAD_MSG("File watcher thread started.");
    while (!shouldStop)
    {
        constexpr int buffer_len{sizeof(inotify_event) + name_max_len + 1};
        char buffer[buffer_len];
        size_t numRead = read(fd, buffer, sizeof(buffer));
        if (numRead < 1)
            std::cerr << "Did not read anything" << '\n';
        auto in_event = reinterpret_cast<inotify_event*>(buffer);
        int wd = in_event->wd;
        curName = in_event->name;
        auto res = watches.findElement(wd, curName);
        if (res) 
        {
            auto event = eventPool.request(wd, curName);
            eventQueue.push(std::move(event));
            std::cout << "Rustle in " << curName<< " detected." << '\n';
        }
    }
    SWD_THREAD_MSG("File watcher thread exitted.");
}

void FileWatcher::stop()
{
    shouldStop = true;
}

}; // namespace event

}; // namespace sword
