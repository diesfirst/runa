#include "window.hpp"
#include "renderer.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "util.hpp"
#include <unistd.h>
#include "event.hpp"

constexpr uint32_t N_FRAMES = 1000000;
Timer myTimer;

void program1(const std::string card_name)
{
	const std::string path{"shaders/tarot/" + card_name + ".spv"};

	Context context;
	XWindow window(600, 900);
	window.open();
	EventHandler eventHander(window);

	Renderer renderer(context, window);
	auto v1 = renderer.loadShader(
			"shaders/tarot/fullscreen_tri.spv", "vert1", ShaderType::vert);
	auto f1 = renderer.loadShader(path, "frag1", ShaderType::frag);
	auto blur = renderer.loadShader(
			"shaders/radialBlur.spv", "osFrag", ShaderType::frag);
	renderer.createGraphicsPipeline("offscreen", v1, f1, "offscreen", false);
	renderer.createGraphicsPipeline("default", v1, blur, "default", false);
	renderer.recordRenderCommandsTest();

	for (int i = 0; i < N_FRAMES; i++) 
	{
		const auto input = eventHander.fetchUserInput(true);
		renderer.setFragmentInput({
				(float)i,
				(float)input.mouseX,
				(float)input.mouseY, 
				input.mButtonDown});
		myTimer.start();
		renderer.render();
		myTimer.end("Render");
	}

	sleep(50);
}
