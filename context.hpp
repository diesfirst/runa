#ifndef CONTEXT_H
#define CONTEXT_H

#define VK_USE_PLATFORM_XCB_KHR

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

	uint32_t pickQueueFamilyIndex(vk::SurfaceKHR surface) const;

	vk::PhysicalDevice physicalDevice;

	vk::Instance instance;


private:
	//physical device properties
	vk::PhysicalDeviceProperties physicalDeviceProperties;
	vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vk::PhysicalDeviceFeatures physicalDeviceFeatures;
	std::vector<vk::ExtensionProperties> instanceExtensionProperties;
	std::vector<vk::QueueFamilyProperties> queueFamilies;
	std::vector<vk::ExtensionProperties> deviceExtensionProperties;
	vk::Device device;
	vk::Queue graphicsQueue;

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

	void setInstanceExtensions(vk::InstanceCreateInfo&);

	void setDeviceExtensions(vk::DeviceCreateInfo&);
};

#endif /* ifndef CONTEXT_H */
