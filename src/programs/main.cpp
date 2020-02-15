#include "../core/commandstate.hpp"
#include <fstream>

int main(int argc, const char *argv[])
{
    auto app = std::make_unique<Application>(800, 800);
    app->run();
    sleep(1);
    return 0;
}
