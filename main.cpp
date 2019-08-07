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

	commander.allocateCommandBuffersForSwapchain(swapchain);
	painter.prepareForBufferPaint();
	commander.setSwapchainImagesToPresent(swapchain);

	EventHandler eventHandler(
			commander,
			mm,
			painter,
			swapchain);

	printFormatsAvailable(swapchain);
	printAlphaComposite(swapchain.surfCaps);

//	painter.addNewLayer();

	paintLoop(
			eventHandler,
			window,
			commander,
			swapchain);

	commander.cleanUp();
	
	return 0;
}
