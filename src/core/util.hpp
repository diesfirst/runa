#ifndef UTIL_H
#define UTIL_H

#include <bitset>
#include <ctime>
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <chrono>

namespace sword
{

class Timer
{
public:
	void start();
	void end(const std::string& funcName = "Non-specified");
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime, endTime;
};

void Timer::start()
{
	startTime = std::chrono::high_resolution_clock::now();
}

void Timer::end(const std::string& funcName)
{
	endTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();
	std::cout 
		<< funcName << " completed in: " << std::endl
		<< elapsedTime
		<< " s" << std::endl;
}

}; // namespace sword
//class Swapchain;
//class Context;
//
//void printMaxDynamicUBOs(vk::Device device);
//
//void printImageCounts(const Swapchain&);
//
//void printCurrentExtent(const Swapchain&);
//
//void printFormatsAvailable(const Swapchain&);
//
//void printSurfaceCapabilities(const Swapchain&);
//
//void printSurfaceCapabilities(const Swapchain&);
//
//void performChecks(const Swapchain&);
//
//void checkBufferMemReqs(const Context& context, const vk::Buffer& buffer);
//
//void printAlphaComposite(const vk::SurfaceCapabilitiesKHR surfCaps);

#endif
