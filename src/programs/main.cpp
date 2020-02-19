#include "../core/commandstate.hpp"
#include <fstream>

int main(int argc, const char *argv[])
{
    bool rec{false}; bool rea{false};
    if (argc == 3) 
    {
        rec = std::atoi(argv[1]);
        rea = std::atoi(argv[2]);
    }
    assert (!(rec && rea) && "only one can be on");
    Application app{800, 800, rec, rea};
    app.run();
    sleep(1);
    return 0;
}
