#include "window.hpp"
#include "renderer.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "util.hpp"
#include <unistd.h>

constexpr int waitTime = 10000;
Timer myTimer;

void program0()
{
	Context context;
	XWindow window(500, 500);
	window.open();
	Renderer renderer(context, window);
	auto v1 = renderer.loadShader("shaders/big_triangle.spv", "vert1", ShaderType::vert);
	auto f1 = renderer.loadShader("shaders/fragCoord.spv", "frag1", ShaderType::frag);
	renderer.createGraphicsPipeline("carl", v1, f1, "default", false);
	renderer.recordRenderCommands("carl", "default");
	myTimer.start();
	renderer.render();
	myTimer.end("Render");
	sleep(50);
}

