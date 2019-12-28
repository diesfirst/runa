#ifndef MEM
#define MEM
#include <vulkan/vulkan.hpp>
#include <vector>

namespace mm
{

class Buffer
{
friend class MemoryManager;
public:
	Buffer(const vk::Device& device);
	Buffer(
			const vk::Device&,
			vk::PhysicalDeviceMemoryProperties,
			uint32_t size,
			vk::BufferUsageFlags,
			vk::MemoryPropertyFlags);
	~Buffer();
	Buffer(const Buffer&) = delete;
	Buffer& operator=(Buffer&) = delete;
	Buffer& operator=(Buffer&&) = delete;
	Buffer(Buffer&&) = delete;
	vk::Buffer& getHandle();
	uint8_t* getPHostMem();
	uint8_t* map();
	void unmap();

private:
	const vk::Device& device;
	vk::DeviceMemory memory;
	vk::Buffer handle;
	void* pHostMemory{nullptr};
	unsigned long size;
	bool isMapped = false;
	vk::PhysicalDeviceMemoryProperties memoryProperties;

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags);
};

class Image
{
friend class MemoryManager;
public:
	Image(const vk::Device& device);
	Image(
			const vk::Device& device, 
			const vk::Image, 
			const vk::Extent3D, 
			const vk::Format, 
			const vk::ImageUsageFlags);
	~Image();
	vk::ImageView& getView();
private:
	vk::Image handle;
	vk::ImageView view;
	vk::DeviceMemory memory;
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
}
#endif
