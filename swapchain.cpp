#include "swapchain.hpp"

Swapchain::Swapchain(const Context& context) :
	context(context)
{
	createSurface();
	queueFamilyIndex = context.pickQueueFamilyIndex(surface);
	setColorFormat();
//	setGraphicsQueue();
//	setPresentQueue();
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

void Swapchain::setColorFormat()
{
	std::vector<vk::SurfaceFormatKHR> surfaceFormats =
		context.physicalDevice.getSurfaceFormatsKHR(surface);
	auto formatCount = surfaceFormats.size();
	vk::SurfaceFormatKHR firstFormat = surfaceFormats[0];
	if ((formatCount == 1) && (firstFormat.format == vk::Format::eUndefined)) colorFormat = vk::Format::eB8G8R8A8Unorm;
	else colorFormat = firstFormat.format;
	colorSpace = firstFormat.colorSpace;
}

void Swapchain::getSurfaceCapabilities()
{
	context.physicalDevice.getSurfaceCapabilitiesKHR(surface);
}

