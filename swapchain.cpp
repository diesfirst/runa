#include "swapchain.hpp"

Swapchain::Swapchain(const Context& context) :
	context(context)
{
	createSurface();
	queueFamilyIndex = context.pickQueueFamilyIndex(surface);
}

Swapchain::~Swapchain()
{
	context.instance.destroySurfaceKHR(surface);
}

void Swapchain::createSurface()
{
	vk::XcbSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.connection  = window.getConnection();
	surfaceCreateInfo.window = window.getWindow();
	surface = context.instance.createXcbSurfaceKHR(surfaceCreateInfo);
}

void Swapchain::setGraphicsQueue(vk::Device device)
{
	graphicsQueue = device.getQueue(queueFamilyIndex, 0); // we'll use first queue
}

void Swapchain::setPresentQueue(vk::Device device)
{
	presentQueue = device.getQueue(queueFamilyIndex, 1); //we'll grab the second queue for this
}
