#include "../core/renderer.hpp"
#include <dlfcn.h>

void run()
{
    auto c = new Context;
    delete c;
    sleep(1);
}

int main(int argc, const char *argv[])
{
    auto lib = dlopen("/usr/lib/x86_64-linux-gnu/libvulkan.so.1.2.131", RTLD_GLOBAL);
    std::cout << "lib: " << lib << std::endl;
    dlclose(lib);
    return 0;
}
