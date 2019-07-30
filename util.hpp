#ifndef UTIL_H
#define UTIL_H

#include <bitset>
#include <ctime>
#include <vulkan/vulkan.hpp>
#include <iostream>

class Swapchain;
class Context;

void printImageCounts(const Swapchain&);

void printCurrentExtent(const Swapchain&);

void printFormatsAvailable(const Swapchain&);

void printSurfaceCapabilities(const Swapchain&);

void printSurfaceCapabilities(const Swapchain&);

void performChecks(const Swapchain&);

void checkBufferMemReqs(const Context& context, const vk::Buffer& buffer);

void printAlphaComposite(const vk::SurfaceCapabilitiesKHR surfCaps);

struct Timer
{
	std::clock_t startTime, endTime;
	void start()
	{
		startTime = std::clock();
	}
	void end()
	{
		endTime = std::clock();
		std::cout << (endTime - startTime) / (double) CLOCKS_PER_SEC << std::endl;
	}
};

#endif
