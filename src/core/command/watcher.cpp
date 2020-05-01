#include "watcher.hpp"
#include <application.hpp>

namespace sword
{

namespace command
{

void WatchFile::execute(Application* app) 
{
    if (app->dispatcher.fileWatcher.addWatch(path.c_str()))
        success();
}

state::Report* WatchFile::makeReport() const
{
    return nullptr;
}

}; // namespace command

}; // namespace sword
