#include "../core/window.hpp"
#include "../core/renderer.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "../core/util.hpp"
#include <unistd.h>
#include "../core/event.hpp"

constexpr uint32_t N_FRAMES = 10000000;
constexpr uint32_t WIDTH = 666;
constexpr uint32_t HEIGHT = 888;

constexpr char SHADER_DIR[] = "build/shaders/";
Timer myTimer;

int main(int argc, char *argv[])
{
    Context context;
    XWindow window(WIDTH, HEIGHT);
    window.open();
    EventHandler eventHander(window);

    for (int i = 0; i < N_FRAMES; i++) 
    {
        myTimer.start();
        auto& input = eventHander.fetchUserInput(true);
        myTimer.end("Got mouse event.");
    }

    sleep(4);
}

