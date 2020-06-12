#ifndef RENDER_RESOURCE_HPP_
#define RENDER_RESOURCE_HPP_

#include <vulkan/vulkan.hpp>
#include <vector>
#include "types.hpp"

namespace sword
{

namespace render
{

class Buffer;

struct BufferBlock
{
    uint32_t offset{0};
    uint32_t size{0};
    uint32_t allocSize{0};
    uint32_t index{0};
    void* pHostMemory{nullptr};
    bool isMapped{false};
    bool isValid{true};
};

class Buffer final
{
friend class MemoryManager;
public:
    Buffer(
            BufferResources,
            uint32_t size,
            vk::BufferUsageFlags,
            vk::MemoryPropertyFlags);
    ~Buffer();
    Buffer(const Buffer&) = delete;
    Buffer& operator=(Buffer&) = delete;
    Buffer& operator=(Buffer&&) = delete;
    Buffer(Buffer&&) = delete;

    vk::Buffer& getHandle();
    BufferBlock* requestBlock(uint32_t size);
    void popBackBlock();
    void map();
    void unmap();
    std::vector<std::unique_ptr<BufferBlock>> bufferBlocks;

private:
    const vk::Device& device;
    const vk::PhysicalDeviceProperties& devProps;
    const vk::PhysicalDeviceMemoryProperties& memProps;
    vk::DeviceMemory memory;
    vk::Buffer handle;
    vk::MemoryPropertyFlags memoryTypeFlags;
    unsigned long size;
    bool isMapped{false};
    void* pHostMemory{nullptr};
    uint32_t curBlockOffset{0};
    void allocateAndBindMemory();
};

class Image final
{
friend class MemoryManager;
public:
    Image(
        const vk::Device& device,
        const vk::Extent3D,
        const vk::Format,
        const vk::ImageUsageFlags,
        const vk::ImageLayout,
        const vk::Filter = vk::Filter::eLinear);
    Image(
        const vk::Device& device, 
        const vk::Image, 
        const vk::Extent3D, 
        const vk::Format, 
        const vk::ImageUsageFlags);
    ~Image();
    Image(const Image&) = delete;
    Image& operator=(Image&) = delete;
    Image& operator=(Image&&) = delete;
    Image(Image&&) = delete;

    const vk::Extent2D getExtent2D() const;
    const vk::ImageView& getView() const;
    const vk::Sampler& getSampler() const;
    vk::Image& getImage();
private:
    vk::Image handle;
    vk::ImageView view;
    vk::DeviceMemory memory;
    vk::Sampler sampler;
    vk::DeviceSize deviceSize;
    vk::Extent3D extent;
    vk::ImageLayout layout;
    vk::ImageUsageFlags usageFlags;
    vk::Format format;
    void* pHostMemory{nullptr};
    bool isMapped = false;
    bool selfManaged = true;
    const vk::Device& device;
};


}; // namespace render

}; // namespace sword

#endif
