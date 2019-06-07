#include "swapchain.hpp"

Swapchain::Swapchain(const Context& context) :
	context(context)
{
	createSurface();
	queueFamilyIndex = context.pickQueueFamilyIndex(surface);
	getSurfaceCapabilities();
	chooseSwapExtent();
	chooseFormat();
	createSwapchain();
}

Swapchain::~Swapchain()
{	
	context.device.destroySwapchainKHR(swapchain);
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
	surfCaps = context.physicalDevice.getSurfaceCapabilitiesKHR(surface);
}

void Swapchain::chooseSwapExtent()
{
	if (surfCaps.currentExtent == -1) 
	{
		swapchainExtent.width = window.size[0];
		swapchainExtent.height = window.size[1];
	}
	else
	{
		swapchainExtent.width = surfCaps.currentExtent.width;
		swapchainExtent.height = surfCaps.currentExtent.height;
	}
}

void Swapchain::chooseFormat()
{
	context.physicalDevice.getSurfaceFormatsKHR(surface);
	colorFormat = vk::Format::eB8G8R8A8Snorm;
}

void Swapchain::checkImageCounts()
{
	int minImageCount = surfCaps.minImageCount;
	int maxImageCount = surfCaps.maxImageCount;
	std::cout << "Min image count = " << minImageCount << std::endl;
	std::cout << "Max image count = " << maxImageCount << std::endl;
}

void Swapchain::checkCurrentExtent()
{
	int width = surfCaps.currentExtent.width;
	int height = surfCaps.currentExtent.height;
	std::cout << "Surf width: " << 
		width << " height: " << 
		height << std::endl;
}

void Swapchain::createSwapchain()
{
	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setSurface(surface);
	createInfo.setImageExtent(swapchainExtent);
	createInfo.setImageFormat(colorFormat);
	createInfo.setImageColorSpace(colorSpace);
	createInfo.setPresentMode(presentMode);
	//request one more image than min to avoid
	//wait times on images
	createInfo.setMinImageCount(surfCaps.minImageCount + 1);
	createInfo.setImageArrayLayers(1); //more than 1 for VR applications
	//we will be drawing directly to the image
	//as opposed to transfering data to it
	createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
	//this is so that each queue has exclusive 
	//ownership of an image at a time
	createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
	createInfo.setPreTransform(surfCaps.currentTransform);
	createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
	//means we dont care about the color of obscured pixels
	createInfo.setClipped(true); 
//	createInfo.setOldSwapchain(nullptr);
	swapchain = context.device.createSwapchainKHR(createInfo);
	std::cout << "Swapchain created!" << std::endl;
}
