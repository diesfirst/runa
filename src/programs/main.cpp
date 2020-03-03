#include "../core/commandstate.hpp"
#include <fstream>

int main(int argc, const char *argv[])
{
    std::string logfile;
    if (argc == 2) 
    {
        const char* file = argv[1];
        logfile = file;
    }
    else
        logfile = "eventlog";
    Application app{800, 800, logfile};
    app.run();
    sleep(1);
    return 0;
}
