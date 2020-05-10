#ifndef COMMAND_SAVEIMAGE_HPP
#define COMMAND_SAVEIMAGE_HPP

#include "command.hpp"

namespace sword
{

namespace command
{

class SaveSwapToPng : public Command
{
public:
    void execute(Application*) override;\
    const char* getName() const override {return "SaveSwapToPng";};
    void set( const std::string_view name) { filename = name; }
    state::Report* makeReport() const override { return nullptr; }
private:
    std::string filename;
};

}; // namespace command

}; // namespace sword

#endif /* end of include guard: COMMAND_SAVEIMAGE_HPP */
