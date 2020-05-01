#include <filesystem>
#include <unistd.h>
#include <iostream>

int main(int argc, const char *argv[])
{
    std::filesystem::path path("/home/michaelb/dev/sword");
    std::cout << path << '\n';
    path.append("src/core/shaders/fragment");
    std::cout << path << '\n';
}
