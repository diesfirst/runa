#include "swapchain.hpp"

int main(int argc, char *argv[])
{
	Context context;
	Swapchain swapchain(context);
	std::cout << "Surface created successfully" << std::endl;
	return 0;
}
