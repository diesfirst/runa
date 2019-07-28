#include "swapchain.hpp"
#include "commander.hpp"
#include "painter.hpp"
#include "window.hpp"
#include "mem.hpp"
#include <thread>
#include <chrono>
#include <ctime>

void paintLoop(
	Window &window, 
	Painter &painter, 
	Commander &commander,
	Swapchain &swapchain)
{
//	std::clock_t start, end;
//	int i = 0;
	while (true)
	{
		window.waitForEvent();
		if (window.mButtonDown)
			painter.paintBuffer(window.mouseX, window.mouseY);
//		window.printMousePosition();
//		start = std::clock();
		commander.renderFrame(swapchain);
//		end = std::clock();
//		std::cout << "Delta = " << (start - end) / (double) CLOCKS_PER_SEC << std::endl;
//		std::cout << "Loop iteration: " << i << std::endl;
//		i++;
	}
}

int main(int argc, char *argv[])
{
	Context context;
	MemoryManager mm(context);

	context.printDeviceMemoryTypeInfo();
	context.printDeviceMemoryHeapInfo();

	Window window;
	window.open();

	Swapchain swapchain(context, window);
	Commander commander(context);

	Painter painter(swapchain, commander, mm);

	commander.allocateCommandBuffersForSwapchain(swapchain);
	painter.prepareForBufferPaint();
	commander.setSwapchainImagesToPresent(swapchain);
	
	paintLoop(window, painter, commander, swapchain);

	commander.cleanUp();
	
	return 0;
}
