#include "swapchain.hpp"

Swapchain::Swapchain(const Context& context) :
	context(context)
{
	createSurface();
}

void Swapchain::createSurface()
{
	vk::XcbSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.connection  = window.getConnection();
	surfaceCreateInfo.window = window.getWindow();
	surface = context.instance.createXcbSurfaceKHR(surfaceCreateInfo);
}
