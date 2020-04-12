#include <command/rendercommands.hpp>
#include <application.hpp>

namespace sword
{

namespace command
{

void OpenWindow::execute(Application* app)
{
    app->window.open();
}

}; // namespace command

}; // namespace sword
