#ifndef COMMAND_WATCHER_HPP
#define COMMAND_WATCHER_HPP

#include "command.hpp"
#include <string>

namespace sword
{

namespace command
{

class WatchFile : public Command
{
public:
    void execute(Application*) override;
    const char* getName() const override {return "WatchFile";};
    void set(std::string path) {this->path = path;}
    state::Report* makeReport() const override;
private:
    std::string path;
};

}; // namespace command

}; // namespace sword

#endif /* end of include guard: COMMAND_WATCHER_HPP */
