#include "window.hpp"
#include "renderer.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "util.hpp"
#include <unistd.h>
#include "event.hpp"

constexpr int waitTime = 10000;
constexpr uint32_t N_FRAMES = 100;
Timer myTimer;

void program0(const std::string card_name)
{
	const std::string path{"shaders/tarot/" + card_name + ".spv"};

	Context context;
	XWindow window(600, 900);
	window.open();
	EventHandler eventHander(window);

	Renderer renderer(context, window);
	auto v1 = renderer.loadShader("shaders/tarot/fullscreen_tri.spv", "vert1", ShaderType::vert);
	auto f1 = renderer.loadShader(path, "frag1", ShaderType::frag);
	renderer.createGraphicsPipeline("carl", v1, f1, "default", false);
	renderer.recordRenderCommands("carl", "default");

	for (int i = 0; i < N_FRAMES; i++) 
	{
		auto input = eventHander.fetchUserInput(true);
		renderer.setFragmentInput(i);
		renderer.render();
	}

	sleep(50);
}

