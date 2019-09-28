#ifndef CONTEXT_H
#define CONTEXT_H

#define VK_USE_PLATFORM_XCB_KHR

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "commander.hpp"
#include "mem.hpp"

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

	vk::Device device;

	bool enableValidation = true;

	uint32_t getGraphicsQueueFamilyIndex() const;
	
	void setQueue();

	BufferBlock* getVertexBlock(uint32_t size);

	void printInstanceExtensionProperties();

	void printDeviceMemoryHeapInfo();

	void printDeviceMemoryTypeInfo();

	void printDeviceInfo();

	void printDeviceFeatures();

	void printDeviceQueueFamilyInfo();

	void printDeviceExtensionProperties();
	
	void printAvailableDevices();

	vk::Queue queue;
	std::unique_ptr<Commander> pCommander;

private:
	std::unique_ptr<MemoryManager> pMemory;
	//physical device properties
	vk::PhysicalDeviceProperties physicalDeviceProperties;
	vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vk::PhysicalDeviceFeatures physicalDeviceFeatures;
	std::vector<vk::ExtensionProperties> instanceExtensionProperties;
	std::vector<vk::QueueFamilyProperties> queueFamilies;
	std::vector<vk::ExtensionProperties> deviceExtensionProperties;
	vk::DispatchLoaderDynamic dispatcher;

	void createContext();

	void destroyContext();

	void createCommander();

	void checkInstanceExtensionProperties();

	void createInstance();

	void createPhysicalDevice();

	void createDevice();

	void getSurfaceCapabilities();

	void setDeviceExtensions(vk::DeviceCreateInfo&);

	void setValidationLayers();

	void setupDebugMessenger();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) 
	{
	    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	    return VK_FALSE;
	}
};

#endif /* ifndef CONTEXT_H */
