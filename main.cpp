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
	painter.fillCanvas();
	painter.paint();

	swapchain.prepareForRender();
	commander.initializeCommandBuffers(swapchain, painter);
	
	//render iteration
	std::chrono::milliseconds interval = 
		std::chrono::milliseconds(500);
	commander.renderFrame(swapchain);
	std::cout << "Frame done" << std::endl;
	std::this_thread::sleep_for(interval);
	commander.renderFrame(swapchain);
	std::cout << "Frame done" << std::endl;
	std::this_thread::sleep_for(interval);
	commander.renderFrame(swapchain);
	std::cout << "Frame done" << std::endl;
	std::this_thread::sleep_for(interval);
	commander.renderFrame(swapchain);
	std::cout << "Frame done" << std::endl;
	std::this_thread::sleep_for(interval);
	commander.renderFrame(swapchain);
	std::cout << "Frame done" << std::endl;
	std::this_thread::sleep_for(interval);
	window.waitForEvent();
	//render end

	commander.cleanUp();
	
	std::cout << std::bitset<32>(UINT32_MAX) << std::endl;
	return 0;
}
