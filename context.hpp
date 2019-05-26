#ifndef CONTEXT_H
#define CONTEXT_H

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <vector>
#include <string>

class Context
{
public:
	Context ();

	virtual ~Context ();

	void deviceReport();

	vk::Device getDevice();

private:
	vk::Instance instance;
	//physical device properties
	vk::PhysicalDevice physicalDevice;
	vk::PhysicalDeviceProperties physicalDeviceProperties;
	vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vk::PhysicalDeviceFeatures physicalDeviceFeatures;
	std::vector<vk::ExtensionProperties> instanceExtensionProperties;
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
	std::vector<vk::ExtensionProperties> deviceExtensionProperties;
	vk::Device device;

	void createContext();

	void destroyContext();

	void checkInstanceExtensionProperties();

	void createInstance();

	void createPhysicalDevice();

	void createDevice();

	void printInstanceExtensionProperties();

	void printDeviceMemoryHeapInfo();

	void printDeviceMemoryTypeInfo();

	void printDeviceInfo();

	void printDeviceFeatures();

	void printDeviceQueueFamilyInfo();

	void printDeviceExtensionProperties();

	void getSurfaceCapabilities();
};

#endif /* ifndef CONTEXT_H */
