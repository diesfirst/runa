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
	queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
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
	for (int i = 0; i < queueFamilyProperties.size(); ++i) {
		const auto& queueFamilyProperty = queueFamilyProperties[i];
		std::cout << "Queue Family: " << i << std::endl;
		std::cout << "Queue Family Flags: " << 
			vk::to_string(queueFamilyProperty.queueFlags) << std::endl;
		std::cout << "QueueCount: " << queueFamilyProperty.queueCount << std::endl;
	}
}

void Context::printDeviceExtensionProperties()
{
	std::cout << "Extension Properties: " << std::endl;
	for (const auto exProp : deviceExtensionProperties) {
		std::cout << "Name: " << exProp.extensionName << std::endl;
	}
}

