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
	while (true)
	{
		commander.renderFrame(swapchain);
		window.waitForEvent();
	}

	commander.cleanUp();
	return 0;
}
