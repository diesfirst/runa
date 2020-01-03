#include "window.hpp"
#include "renderer.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "util.hpp"
#include <unistd.h>
#include "event.hpp"

constexpr uint32_t N_FRAMES = 10000000;
Timer myTimer;

void program1(const std::string card_name)
{
	const std::string path{"shaders/tarot/" + card_name + ".spv"};

	Context context;
	XWindow window(1500, 900);
	window.open();
	EventHandler eventHander(window);

	Renderer renderer(context, window);

    renderer.prepare(path);
    renderer.recordRenderCommandsTest3();

    auto t0 = std::chrono::high_resolution_clock::now();

    renderer.render();
	for (int i = 0; i < N_FRAMES; i++) 
	{
		const auto input = eventHander.fetchUserInput(true);
        if (input.mButtonDown)
        {
            auto t1 = std::chrono::high_resolution_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(t1 - t0).count();
            renderer.setFragmentInput({
                    elapsedTime,
                    (float)input.mouseX,
                    (float)input.mouseY, 
                    input.blur,
                    input.r,
                    input.g,
                    input.b});
            myTimer.start();
            renderer.render();
            myTimer.end("Render");
        }
	}

	sleep(50);
}
