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
	MemoryManager mm(context);

	context.printDeviceMemoryTypeInfo();
	context.printDeviceMemoryHeapInfo();

	XWindow window;
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

	Pipe pipe(context);

	pipe.createGraphicsPipeline();

	commander.cleanUp();
	
	return 0;
}
