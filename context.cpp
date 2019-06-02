#include "context.hpp"

//public

Context::Context()
{
	createContext();
}

Context::~Context()
{
	destroyContext();
}

void Context::deviceReport()
{
	printDeviceInfo();
	printDeviceMemoryHeapInfo();
	printDeviceMemoryTypeInfo();
	printDeviceQueueFamilyInfo();
//	printInstanceExtensionProperties();
	printDeviceExtensionProperties();
}

vk::Device Context::getDevice()
{
	return device;
}

//private

void Context::createContext()
{
	checkInstanceExtensionProperties();
	printInstanceExtensionProperties();
	createInstance();
	createPhysicalDevice();
	createDevice();
}

void Context::destroyContext()
{
	instance.destroy();
}

void Context::createInstance()
{
	vk::InstanceCreateInfo createInstanceInfo;
	const char* names[2];
	names[0] = VK_KHR_SURFACE_EXTENSION_NAME;
	names[1] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
//	setInstanceExtensions(createInstanceInfo);
	createInstanceInfo.enabledExtensionCount = 2;
	createInstanceInfo.ppEnabledExtensionNames = names;
	instance = vk::createInstance(createInstanceInfo);
}

void Context::checkInstanceExtensionProperties()
{
	instanceExtensionProperties = vk::enumerateInstanceExtensionProperties();
}

void Context::createPhysicalDevice()
{
	physicalDevice = instance.enumeratePhysicalDevices()[0];
	physicalDeviceProperties = physicalDevice.getProperties();
	physicalDeviceMemoryProperties = physicalDevice.getMemoryProperties();
	physicalDeviceFeatures = physicalDevice.getFeatures();
	queueFamilies = physicalDevice.getQueueFamilyProperties();
	deviceExtensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
}

void Context::createDevice()
{
	float queuePriority = 1.0; //apparently necesary? 
	vk::DeviceQueueCreateInfo queueInfo;
	queueInfo.queueCount = 1;
	queueInfo.queueFamilyIndex = 0; //should be the one with graphics capabilities
	queueInfo.pQueuePriorities = &queuePriority;
	
	vk::DeviceCreateInfo deviceInfo;
	deviceInfo.pEnabledFeatures = &physicalDeviceFeatures;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;

//	setDeviceExtensions(deviceInfo);
	const uint8_t extCount = 1;
	const char* names[extCount];
	names[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	deviceInfo.enabledExtensionCount = extCount;
	deviceInfo.ppEnabledExtensionNames = names;

	device = physicalDevice.createDevice(deviceInfo);
}

void Context::printInstanceExtensionProperties()
{
	std::cout << "---Instance Extension Properties---" << std::endl;
	for (const auto n : instanceExtensionProperties) {
		std::cout << "Instance extension name: " << n.extensionName << std::endl;
	}
}

void Context::printDeviceMemoryHeapInfo()
{
	std::cout << "Memory Heaps: " << std::endl;
	int heapCount = physicalDeviceMemoryProperties.memoryHeapCount;
	for (int i = 0; i < heapCount; ++i) {
		const auto& heap = physicalDeviceMemoryProperties.memoryHeaps[i];
		std::cout << 
			"Heap: " << i << 
			"\tflags: " << vk::to_string(heap.flags) << 
			"\tSize: " << heap.size << std::endl;
	}
}

void Context::printDeviceMemoryTypeInfo()
{
	std::cout << "Memory types: " << std::endl;
	int typeCount = physicalDeviceMemoryProperties.memoryTypeCount;
	for (int i = 0; i < typeCount; ++i) {
		const auto& type = physicalDeviceMemoryProperties.memoryTypes[i];
		std::cout << 
		       "Type: " << i <<
		       " flags " << vk::to_string(type.propertyFlags) <<
		       " heap " << type.heapIndex << std::endl;
	}
}

void Context::printDeviceInfo()
{
	std::cout << "Device Name: " << physicalDeviceProperties.deviceName << std::endl;
}

void Context::printDeviceFeatures()
{

}


void Context::printDeviceQueueFamilyInfo()
{
	std::cout << "Queues: "	<< std::endl;
	for (int i = 0; i < queueFamilies.size(); ++i) {
		const auto& queueFamilyProperties = queueFamilies[i];
		std::cout << "Queue Family: " << i << std::endl;
		std::cout << "Queue Family Flags: " << 
			vk::to_string(queueFamilyProperties.queueFlags) << std::endl;
		std::cout << "QueueCount: " << queueFamilyProperties.queueCount << std::endl;
	}
}

void Context::printDeviceExtensionProperties()
{
	std::cout << "Extension Properties: " << std::endl;
	for (const auto exProp : deviceExtensionProperties) {
		std::cout << "Name: " << exProp.extensionName << std::endl;
	}
}

uint32_t Context::pickQueueFamilyIndex(vk::SurfaceKHR surface) const
{
	uint32_t index = 0;
	vk::QueueFamilyProperties familyProperties = queueFamilies[0];
	const vk::QueueFlags& desiredFlags = vk::QueueFlagBits::eGraphics;
	if (familyProperties.queueFlags & desiredFlags)
	{
		std::cout << "Is graphics suitable" << std::endl;
	}
	if (physicalDevice.getSurfaceSupportKHR(index, surface))
	{
		std::cout << "Is presentation suitable" << std::endl;
	}

}

void Context::setInstanceExtensions(vk::InstanceCreateInfo& createInfo)
{
	const uint8_t extCount = 0;
	const char* names[extCount];
	names[0] = VK_KHR_SURFACE_EXTENSION_NAME;
	names[1] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
	createInfo.enabledExtensionCount = extCount;
	createInfo.ppEnabledExtensionNames = names;
}

void Context::setDeviceExtensions(vk::DeviceCreateInfo& createInfo)
{
	const uint8_t extCount = 1;
	const char* names[extCount];
	names[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	createInfo.enabledExtensionCount = extCount;
	createInfo.ppEnabledExtensionNames = names;
}


