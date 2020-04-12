#ifndef COMMAND_OPENWINDOW_HPP
#define COMMAND_OPENWINDOW_HPP

#include <command/command.hpp>

namespace sword
{

class Application;

namespace command
{

class OpenWindow : public Command
{
public:
    void execute(Application* app) override;
    const char* getName() const override { return "open_window"; }
private:
};

}; // namespace command

}; // namespace sword


#endif /* end of include guard: COMMAND_OPENWINDOW_HPP */

