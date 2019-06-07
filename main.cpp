#include "swapchain.hpp"

int main(int argc, char *argv[])
{
	Context context;
	context.setGraphicsQueue();
	context.setPresentQueue();
	Swapchain swapchain(context);
	std::cout << "No errors" << std::endl;
	return 0;
}
