#include "util.hpp"

void printImageCounts(const Swapchain& swapchain)
{
	int minImageCount = swapchain.surfCaps.minImageCount;
	int maxImageCount = swapchain.surfCaps.maxImageCount;
	std::cout << "Min image count = " << minImageCount << std::endl;
	std::cout << "Max image count = " << maxImageCount << std::endl;
}

void printCurrentExtent(const Swapchain& swapchain)
{
	int width = swapchain.swapchainExtent.width;
	int height = swapchain.swapchainExtent.height;
	std::cout << "Surf width: " << 
		width << " height: " << 
		height << std::endl;
}

void printFormatsAvailable(const Swapchain& swapchain)
{
	std::vector<vk::SurfaceFormatKHR> surfaceFormats =
		swapchain.context.physicalDevice.getSurfaceFormatsKHR(swapchain.surface);
	for (const auto format : surfaceFormats) {
		std::cout << "Format: " << (uint32_t)format.format 
			<< " Colorspace: " << (uint32_t)format.colorSpace 
			<< std::endl;
	}
}

void printSurfaceCapabilities(const Swapchain& swapchain)
{
	const auto suf = swapchain.surfCaps.supportedUsageFlags;
	std::cout << vk::to_string(suf) << std::endl;
}

void printPresentModes(const Swapchain& swapchain)
{
	std::vector<vk::PresentModeKHR> availablePresentModes;
	availablePresentModes = 
		swapchain.context.physicalDevice.getSurfacePresentModesKHR(
				swapchain.surface);
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
		if (mode == vk::PresentModeKHR::eImmediate)
		{
			std::cout << "Immediate mode at index " << i << std::endl;
		}
	}
}

void performChecks(const Swapchain& swapchain)
{
	printFormatsAvailable(swapchain);
	printPresentModes(swapchain);
}
