#include <application.hpp>
#include <fstream>

int main(int argc, const char *argv[])
{
    std::string logfile{"eventlog"};
    int popEvents{0};
    std::cout << "Arg count is " << argc << std::endl;
    if (argc >= 2) 
    {
        const char* file = argv[1];
        logfile = file;
        std::cout << "Logfile is " << logfile << std::endl;
    }
    if (argc == 3)
    {
        popEvents = atoi(argv[2]);
        std::cout << "Pop events: " << popEvents << std::endl;
    }
    sword::Application app{800, 800, logfile, popEvents};
    app.run();
    return 0;
}
