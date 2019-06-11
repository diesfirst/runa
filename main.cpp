#include "swapchain.hpp"
#include "commander.hpp"
#include <thread>
#include <chrono>

int main(int argc, char *argv[])
{
	Context context;

	Window window;
	window.open();
	window.waitForEvent();

	Swapchain swapchain(context, window);
	Commander commander(context);
	swapchain.prepareForRender();
	commander.initializeCommandBuffers(swapchain);
	

	std::cout << "about to render" << std::endl;
	commander.renderFrame(swapchain);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	commander.renderFrame(swapchain);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	commander.renderFrame(swapchain);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	commander.renderFrame(swapchain);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	commander.renderFrame(swapchain);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	commander.renderFrame(swapchain);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	commander.renderFrame(swapchain);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	commander.cleanUp();
	return 0;
}
