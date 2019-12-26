#ifndef UTIL_H
#define UTIL_H

#include <bitset>
#include <ctime>
#include <vulkan/vulkan.hpp>
#include <iostream>

class Swapchain;
class Context;

void printMaxDynamicUBOs(vk::Device device);

void printImageCounts(const Swapchain&);

void printCurrentExtent(const Swapchain&);

void printFormatsAvailable(const Swapchain&);

void printSurfaceCapabilities(const Swapchain&);

void printSurfaceCapabilities(const Swapchain&);

void performChecks(const Swapchain&);

void checkBufferMemReqs(const Context& context, const vk::Buffer& buffer);

void printAlphaComposite(const vk::SurfaceCapabilitiesKHR surfCaps);

class Timer
{
public:
	void start();
	void end(const std::string& funcName = "Non-specified");
private:
	std::clock_t startTime, endTime;
};

void Timer::start()
{
	startTime = std::clock();
}

void Timer::end(const std::string& funcName)
{
	endTime = std::clock();
	std::cout 
		<< funcName << " completed in: " << std::endl
		<< (endTime - startTime) / (double) CLOCKS_PER_SEC 
		<< " s" << std::endl;
}

#endif
