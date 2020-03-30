#include <render/context.hpp>

namespace sword
{

namespace render
{

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) 
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

Context::Context()
{
    createInstance();
    if (enableValidation) setupDebugMessenger2();
    createPhysicalDevice();
    createDevice();
    setQueue();
}

Context::~Context()
{
    std::cout << "context destructor called" << std::endl;
    device.destroy();
    if (enableValidation) destroyDebugMessenger();
    instance.destroy();
}

void Context::deviceReport()
{
    printDeviceInfo();
    printDeviceMemoryHeapInfo();
    printDeviceMemoryTypeInfo();
    printDeviceQueueFamilyInfo();
//	printInstanceExtensionProperties();
    printDeviceExtensionProperties();
    printDeviceMemoryHeapInfo();
    printDeviceMemoryTypeInfo();
}

vk::Device Context::getDevice()
{
    return device;
}

//private

void Context::createInstance()
{
    std::vector<const char*> extensions;
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);

    std::vector<const char*> layers;
    if (enableValidation)
    {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }
//	layers.push_back("VK_LAYER_LUNARG_api_dump");

    checkLayers(layers);

    vk::InstanceCreateInfo instanceInfo;
    instanceInfo.enabledExtensionCount = extensions.size();
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    instanceInfo.enabledLayerCount = layers.size();
    instanceInfo.ppEnabledLayerNames = layers.data();

    instance = vk::createInstance(instanceInfo);
}

void Context::createPhysicalDevice()
{
    physicalDevice = instance.enumeratePhysicalDevices()[0];
    physicalDeviceProperties = physicalDevice.getProperties();
    physicalDeviceMemoryProperties = physicalDevice.getMemoryProperties();
    physicalDeviceFeatures = physicalDevice.getFeatures();
    physicalDeviceFeatures.setFillModeNonSolid(true);
    physicalDeviceFeatures.setSampleRateShading(true);
    physicalDeviceFeatures.setShaderSampledImageArrayDynamicIndexing(true);
    queueFamilies = physicalDevice.getQueueFamilyProperties();
    deviceExtensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
}

void Context::createDevice()
{
    float queuePriority = 1.0; //apparently necesary? 
    vk::DeviceQueueCreateInfo queueInfo;
    queueInfo.flags = vk::DeviceQueueCreateFlags();
    queueInfo.queueCount = 1;
    queueInfo.queueFamilyIndex = 0; //should be the one with graphics capabilities
    queueInfo.pQueuePriorities = &queuePriority;
    
    vk::DeviceCreateInfo deviceInfo;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;

    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    extensions.push_back(VK_NV_FILL_RECTANGLE_EXTENSION_NAME);
    extensions.push_back(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    deviceInfo.enabledExtensionCount = extensions.size();
    deviceInfo.ppEnabledExtensionNames = extensions.data();
    deviceInfo.setPEnabledFeatures(&physicalDeviceFeatures);

    auto formatProps = physicalDevice.getFormatProperties(vk::Format::eR8G8B8A8Unorm);
    std::cout << "formatProps " << vk::to_string(formatProps.linearTilingFeatures) << std::endl;

    vk::PhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures;
//    vk::PhysicalDeviceFeatures2 features2;
//    features2.setPNext(&indexingFeatures);
//    physicalDevice.getPFeatures2(features2);
//
//    if (indexingFeatures.descriptorBindingPartiallyBound && 
//            indexingFeatures.runtimeDescriptorArray)
//    {
//        std::cout << "Runtime descriptor array is supported" << std::endl;
//    }
//
    indexingFeatures.setRuntimeDescriptorArray(true);
    indexingFeatures.setDescriptorBindingPartiallyBound(true);

    deviceInfo.setPNext(&indexingFeatures);

    device = physicalDevice.createDevice(deviceInfo);
    assert(device && "Device failed to be created");
    std::cout << "Device created at: " << device << std::endl;
}

void Context::setQueue()
{
    //this should be based on the surface requirements, but 0 works
    queue = device.getQueue(0, 0);
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
    for (int i = 0; i < typeCount; ++i) 
    {
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

void Context::printDeviceQueueFamilyInfo()
{
    std::cout << "Queues: "	<< std::endl;
    for (int i = 0; i < queueFamilies.size(); ++i) 
    {
        const auto& queueFamilyProperties = queueFamilies[i];
        std::cout << "Queue Family: " << i << std::endl;
        std::cout << "Queue Family Flags: " << 
            vk::to_string(queueFamilyProperties.queueFlags) << std::endl;
        std::cout << "QueueCount: " << queueFamilyProperties.queueCount << std::endl;
    }
}

void Context::printDeviceExtensionProperties()
{
    std::cout << "Device Extension Properties: " << std::endl;
    for (const auto exProp : deviceExtensionProperties) 
        std::cout << "Name: " << exProp.extensionName << std::endl;
}

uint32_t Context::pickQueueFamilyIndex(vk::SurfaceKHR surface) const
{
    uint32_t index = 0;
    vk::QueueFamilyProperties familyProperties = queueFamilies[index];
    const vk::QueueFlags& desiredFlags = vk::QueueFlagBits::eGraphics;
    if (familyProperties.queueFlags & desiredFlags)
    {
        std::cout << 
                "QueueFamily index " <<
                index <<
                " is graphics suitable" << 
                std::endl;
    }
    if (physicalDevice.getSurfaceSupportKHR(index, surface))
    {
        std::cout << 
                "QueueFamily index " <<
                index <<
                " is presentation suitable" << 
                std::endl;
    }
    return index;
}

uint32_t Context::getGraphicsQueueFamilyIndex() const
{
    return 0;
}

void Context::setDeviceExtensions(vk::DeviceCreateInfo& createInfo)
{
    const uint8_t extCount = 1;
    const char* names[extCount];
    names[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    createInfo.enabledExtensionCount = extCount;
    createInfo.ppEnabledExtensionNames = names;
}

void Context::setupDebugMessenger()
{
    vk::DebugUtilsMessengerCreateInfoEXT dbCreateInfo;
    dbCreateInfo.setMessageSeverity(
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo);
    dbCreateInfo.setMessageType(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    dbCreateInfo.setPfnUserCallback(debugCallback);
    std::cout << "going for it" << std::endl;
    dispatcher.init(instance);
    std::cout << "mm" << std::endl;
    auto messenger = instance.createDebugUtilsMessengerEXTUnique(
                    dbCreateInfo,
                    nullptr,
                    dispatcher);
    std::cout << "Made it" << std::endl;
}

void Context::setupDebugMessenger2()
{
    VkDebugUtilsMessengerCreateInfoEXT dbCreateInfo;
    dbCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dbCreateInfo.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    dbCreateInfo.messageType = 
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    dbCreateInfo.flags = 0;
    dbCreateInfo.pfnUserCallback = debugCallback;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");
    assert(func != nullptr);
    func(instance, &dbCreateInfo, nullptr, &debugMessenger);
}

void Context::destroyDebugMessenger()
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT");
    assert(func != nullptr);
    func(instance, debugMessenger, nullptr);
}

void Context::printAvailableDevices()
{
    for (const auto device : instance.enumeratePhysicalDevices()) 
    {
        std::string name = device.getProperties().deviceName;
        std::cout << "Device name: " << name << std::endl;
    }
}

void Context::checkLayers(std::vector<const char*> layers)
{
    auto layerProperties = vk::enumerateInstanceLayerProperties();
    for (auto layerProp : layerProperties) 
        std::cout << layerProp.layerName << std::endl;
    for (auto layerName : layers) 
    {
        bool layerFound = false;
        for (auto layerProp : layerProperties) 
            if (strcmp(layerName, layerProp.layerName) == 0) 
            {
                    layerFound = true;
                    break;
            }
        if (!layerFound) 
            throw std::runtime_error("layer requested but not found");
    }
}


}; // namespace render

}; // namespace sword


