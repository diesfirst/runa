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
    std::cerr << "FileWatcher::addWatch: Adding watch to dir:" << parent.c_str() << '\n';
    int watch = inotify_add_watch(fd, parent.c_str(), IN_CLOSE_WRITE);
    if (watch == -1)
        return false;
    std::string name = path.filename();
    watches.push_back({watch, path_str, name});
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
        std::cerr << "FileWatcher::run: read something..." << '\n';
        for (const auto& w : watches) 
        {
            std::cerr << "w.wd: " << w.wd << '\n';
            std::cerr << "in_event->wd: " << in_event->wd << '\n';
            std::cerr << "w.filename: " << w.filename << '\n';
            std::cerr << "in_event->name: " << in_event->name  << '\n';

            if (w.wd == in_event->wd && w.filename == in_event->name)
            {
                auto event = eventPool.request(w.wd, w.fullpath);
                eventQueue.push(std::move(event));
                std::cout << "Rustle in " << w.filename << " detected." << '\n';
            }
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
