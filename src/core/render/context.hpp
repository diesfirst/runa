#ifndef CONTEXT_H
#define CONTEXT_H

#define VK_USE_PLATFORM_XCB_KHR

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <memory>

namespace sword
{

namespace render
{

struct QueueInfo
{
    QueueInfo(int familyIndex, vk::QueueFlags flags, int queueCount) :
        familyIndex{familyIndex},
        flags{flags},
        queueCount{queueCount},
        priorites(queueCount, 1.0) //all queues have a priority of one
    {}
    vk::DeviceQueueCreateInfo makeCreateInfo()
    {
        vk::DeviceQueueCreateInfo ci;
        ci.setQueueFamilyIndex(familyIndex);
        ci.setQueueCount(queueCount);
        ci.setPQueuePriorities(priorites.data());
        return ci;
    }
    int familyIndex;
    vk::QueueFlags flags;
    int queueCount;
    std::vector<float> priorites;
};

class Context
{
public:
    Context();
    ~Context();
    void deviceReport();
    vk::Device getDevice();
    uint32_t pickQueueFamilyIndex(vk::SurfaceKHR surface) const;
    vk::PhysicalDevice physicalDevice;
    vk::Instance instance;
    vk::Device device;
    bool enableValidation = true;
    uint32_t getGraphicsQueueFamilyIndex() const;
    void printDeviceMemoryHeapInfo();

    void printDeviceMemoryTypeInfo();

    void printDeviceInfo();

    void printDeviceQueueFamilyInfo();

    void printDeviceExtensionProperties();
    
    void printAvailableDevices();

    void checkLayers(std::vector<const char*>);

    //TODO should use these for getting queues
    vk::Queue getGraphicQueue(int index) const;

    vk::Queue getTransferQueue(int index) const;

    //physical device properties
    vk::PhysicalDeviceProperties physicalDeviceProperties;
    vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

private:
    vk::PhysicalDeviceFeatures physicalDeviceFeatures;
    std::optional<QueueInfo> graphicsQueueInfo;
    std::optional<QueueInfo> computeQueueInfo;
    std::optional<QueueInfo> transferQueueInfo;
    std::vector<vk::QueueFamilyProperties> queueFamilies;
    std::vector<vk::ExtensionProperties> deviceExtensionProperties;
    VkDebugUtilsMessengerEXT debugMessenger;
    vk::DispatchLoaderDynamic dispatcher;

    void setupDebugMessenger2();

    void destroyDebugMessenger();

    void destroyContext();

    void createCommander();

    void createInstance();

    void createPhysicalDevice();

    void createDevice();

    void getSurfaceCapabilities();

    void setDeviceExtensions(vk::DeviceCreateInfo&);

    void setValidationLayers();

    void setupDebugMessenger();
};

}; // namespace render

}; // namespace sword


#endif /* ifndef CONTEXT_H */
