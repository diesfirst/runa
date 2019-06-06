#include "swapchain.hpp"

Swapchain::Swapchain(const Context& context) :
	context(context)
{
	createSurface();
	queueFamilyIndex = context.pickQueueFamilyIndex(surface);
	setColorFormat();
}

Swapchain::~Swapchain()
{
	context.instance.destroySurfaceKHR(surface);
}

void Swapchain::checkPresentModes()
{
	std::vector<vk::PresentModeKHR> availablePresentModes;
	availablePresentModes = context.physicalDevice.getSurfacePresentModesKHR(surface);
	int numberOfModes = availablePresentModes.size();
	std::cout << "Number of present modes: " << numberOfModes << std::endl;
	for (int i = 0; i < numberOfModes; ++i)
	{
		const auto mode = availablePresentModes[i];
		if (mode == vk::PresentModeKHR::eFifo) 
		{
			std::cout << "Fifo mode available at index " << 
				i << std::endl;
		}
		if (mode == vk::PresentModeKHR::eMailbox) 
		{
			std::cout << "Mailbox mode at index" <<
			        i << std::endl;	
		}
	}
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


