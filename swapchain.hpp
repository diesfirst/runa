#include "context.hpp"
#include "window.hpp"

class Swapchain
{
public:
	Swapchain (const Context& context);

private:
	const Context& context;
	vk::SurfaceKHR surface;
	Window window;
	
	void createSurface();
};
