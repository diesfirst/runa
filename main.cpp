#include "swapchain.hpp"
#include "commander.hpp"
#include "painter.hpp"
#include <thread>
#include <chrono>

int main(int argc, char *argv[])
{
	Context context;

	context.printDeviceMemoryTypeInfo();
	context.printDeviceMemoryHeapInfo();

	Window window;
	window.open();
	window.waitForEvent();

	Swapchain swapchain(context, window);

	Painter painter(swapchain);

	Commander commander(context);

	painter.prepare();

	swapchain.prepareForRender();
	commander.initializeCommandBuffers(swapchain, painter);
	

	std::cout << "about to render" << std::endl;
	while (true)
	{
		window.waitForEvent();
		painter.paint();
		commander.renderFrame(swapchain);
	}

	commander.cleanUp();
	
	std::cout << std::bitset<32>(UINT32_MAX) << std::endl;
	return 0;
}
