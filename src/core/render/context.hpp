#ifndef CONTEXT_H
#define CONTEXT_H

#define VK_USE_PLATFORM_XCB_KHR

#include <types/vktypes.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "types.hpp"

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
    Context(Context&&) = delete;
    Context(const Context&) = delete;
    Context& operator=(Context&) = delete;
    Context& operator=(Context&&) = delete;
    void deviceReport();
    const vk::Device& getDevice() const;
    const vk::Instance& getInstance() const;
    const vk::PhysicalDevice& getPhysicalDevice() const;
    uint32_t pickQueueFamilyIndex(vk::SurfaceKHR surface) const;
    bool enableValidation = true;
    uint32_t getGraphicsQueueFamilyIndex() const;
    uint32_t getTransferQueueFamilyIndex() const { return transferQueueInfo->familyIndex; }
    void printDeviceMemoryHeapInfo();

    void printDeviceMemoryTypeInfo();

    void printDeviceInfo();

    void printDeviceQueueFamilyInfo();

    void printDeviceExtensionProperties();
    
    void printAvailableDevices();

    BufferResources getBufferResources() const;

    void checkLayers(std::vector<const char*>);

    //TODO should use these for getting queues
    vk::Queue getGraphicQueue(int index) const;

    vk::Queue getTransferQueue(int index) const;

    //physical device properties
    vk::PhysicalDeviceProperties physicalDeviceProperties;
    vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

private:
    vk::PhysicalDevice physicalDevice;
    vk::UniqueInstance instance;
    vk::UniqueDevice device;

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
