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
#include "sculpter.hpp"
#include "camera.hpp"
#include "viewport.hpp"

constexpr int WIDTH = 800;
constexpr int HEIGHT = 800;

Context context;
MemoryManager mm(context);
Commander commander(context);
Viewport viewport(WIDTH, HEIGHT, context);
Painter painter(commander, mm);
Sculpter sculpter;
Pipe pipeliner(context);
Renderer renderer(context, pipeliner);
Camera camera(WIDTH, HEIGHT);
EventHandler eventHandler(
		commander,
		mm,
		painter,
		swapchain,
		sculpter,
		renderer,
		pipeliner,
		camera);

void paintLoop(
		EventHandler& eventHandler, 
		XWindow& window)
{
	while (true)
	{
		eventHandler.handleEvent(window.waitForEvent());
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

	paintLoop(eventHandler, window);
}

int runSculpt()
{
	window.open();

	commander.allocateCommandBuffersForSwapchain(swapchain);

	sculpter.aquireBlock(mm);
	sculpter.createGeo();

	pipeliner.createGraphicsPipeline(renderer.renderPass, WIDTH, HEIGHT);
	renderer.createFramebuffers(swapchain);

	camera.acquireUniformBufferBlocks(mm, 3);
	camera.copyXFormsToBuffers();

	pipeliner.prepareDescriptors(3);
	pipeliner.updateDescriptorSets(3, camera.uboBlocks, sizeof(ViewTransforms));

	eventHandler.setState(EventState::sculpt);
	
	std::cout << "Event state " << static_cast<int>(eventHandler.state) << std::endl;

	while (true)
		eventHandler.handleEvent(window.waitForEvent());

	commander.cleanUp();
}
