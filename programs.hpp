#include "window.hpp"
#include "renderer.hpp"
#include <thread>
#include <chrono>
#include <random>
#include "util.hpp"

constexpr int waitTime = 10000;
Timer myTimer;

void program0()
{
	Context context;
	XWindow window(500, 500);
	window.open();
	std::cout << "Device is at " << context.device << std::endl;
	context.device.waitIdle();
	Renderer renderer(context, window);
	auto v1 = renderer.loadShader("shaders/triangle.spv", "vert1", ShaderType::vert);
	auto f1 = renderer.loadShader("shaders/simpleFrag.spv", "frag1", ShaderType::frag);
//	renderer.createGraphicsPipeline("carl", v1, f1, "default", false);
}

