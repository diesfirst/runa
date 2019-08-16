#include "swapchain.hpp"
#include "commander.hpp"
#include "painter.hpp"
#include "window.hpp"
#include "mem.hpp"
#include "io.hpp"
#include "event.hpp"
#include <thread>
#include <chrono>
#include <ctime>
#include "util.hpp"
#include "pipe.hpp"
#include "renderer.hpp"
#include "geo.hpp"

constexpr int WIDTH = 800;
constexpr int HEIGHT = 800;

void paintLoop(
		EventHandler& eventHandler, 
		XWindow& window, 
		Commander& commander,
		Swapchain& swapchain)
{
	Timer timer;
	while (true)
	{
//		timer.start();
		eventHandler.handleEvent(window.waitForEvent());
//		timer.end();
	}
}

void runPaintProgram(
		Painter& painter,
		Commander& commander,
		Swapchain& swapchain,
		EventHandler& eventHandler,
		XWindow& window)
{
	painter.prepareForBufferPaint();
	commander.setSwapchainImagesToPresent(swapchain);

	paintLoop(
			eventHandler,
			window,
			commander,
			swapchain);
}

int main(int argc, char *argv[])
{
	Context context;


	vk::VertexInputBindingDescription desc = Geo::getBindingDescription();
	std::cout << "Stride: " << desc.stride << std::endl;



	MemoryManager mm(context);

	context.printDeviceMemoryTypeInfo();
	context.printDeviceMemoryHeapInfo();

	XWindow window(WIDTH, HEIGHT);
	window.open();

	Swapchain swapchain(context, window);
	Commander commander(context);
	Painter painter(swapchain, commander, mm);
	EventHandler eventHandler(
			commander,
			mm,
			painter,
			swapchain);

	commander.allocateCommandBuffersForSwapchain(swapchain);
	//
	//run program
//	runPaintProgram(painter, commander, swapchain, eventHandler, window);

	Triangle triangle;
	bufferBlock* block = mm.vertexBlock(triangle);
	triangle.printPoints();

	Pipe pipe(context);
	Renderer renderer(context);
	pipe.createGraphicsPipeline(renderer, WIDTH, HEIGHT);
	renderer.createFramebuffers(swapchain);

	commander.recordRenderpass(renderer.renderPass, pipe.graphicsPipeline, renderer.framebuffers, block->buffer, WIDTH, HEIGHT);

	commander.renderFrame(swapchain);
	std::this_thread::sleep_for(std::chrono::minutes(1));

	commander.cleanUp();
	
	return 0;
}
