#include "mem.hpp"
#include <cstring>
#include <iostream>

namespace mm
{

Buffer::Buffer(const vk::Device& device) :
	device(device)
{
}

Buffer::Buffer(const vk::Device& device,
		vk::PhysicalDeviceMemoryProperties memProps,
		uint32_t size, 
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags typeFlags) :
	device(device),
	size{size}
{
	std::cout << "Creating buffer!" << std::endl;
	std::cout << "MM Size: " << size << std::endl;

	vk::BufferCreateInfo bufferInfo;
	bufferInfo.setSize(size);
	bufferInfo.setUsage(usage);
	bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

	handle = device.createBuffer(bufferInfo);

	auto memReqs = device.getBufferMemoryRequirements(handle);
//	to do: fix this, its not working
//	uint32_t memoryTypeIndex = findMemoryType(
//			memReqs.memoryTypeBits, typeFlags); 
//	std::cout << "Memory Type index: " << memoryTypeIndex << std::endl;
//
	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setMemoryTypeIndex(9); //always host visible for now
	allocInfo.setAllocationSize(memReqs.size);
//	allocInfo.setMemoryTypeIndex(memoryTypeIndex);
	std::cout << "Mem reqs size:" << memReqs.size << std::endl;
	std::cout << "Mem reqs alignment:" << memReqs.alignment << std::endl;

	memory = device.allocateMemory(allocInfo);

	device.bindBufferMemory(handle, memory, 0);
}


Buffer::~Buffer()
{
	device.unmapMemory(memory);
	device.destroyBuffer(handle);
	device.freeMemory(memory);
}

vk::Buffer& Buffer::getHandle()
{
	return handle;
}

uint8_t* Buffer::getPHostMem()
{
	assert(isMapped && "Memory not mapped");
	return static_cast<uint8_t*>(pHostMemory);
}

uint8_t* Buffer::map()
{
	assert(!isMapped);
	void* res = device.mapMemory(memory, 0, size);
	assert(res != (void*)VK_ERROR_MEMORY_MAP_FAILED);
	pHostMemory = res;
	isMapped = true;
	return static_cast<uint8_t*>(pHostMemory);
}

void Buffer::unmap()
{
	assert(isMapped);
	device.unmapMemory(memory);
	pHostMemory = nullptr;
	isMapped = false;
}

uint32_t Buffer::findMemoryType(
		uint32_t typeFilter, 
		vk::MemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) 
	{
		if (
			(typeFilter & (1 << i)) && 
			(memoryProperties.memoryTypes[i].propertyFlags & properties) == properties
			) 
		{
		return i;
	    }
	}

	throw std::runtime_error("Failed to find suitable memory");
}

Image::Image(const vk::Device& device) :
	device(device)
{
}

Image::Image(	
		const vk::Device& device, 
		const vk::Image handle, 
		const vk::Extent3D extent, 
		vk::Format format, 
		vk::ImageUsageFlags usageFlags) :
	device(device),
	handle{handle},
	extent(extent),
	format(format),
	usageFlags(usageFlags)
{
	selfManaged = false;
	vk::ComponentMapping components;
	components.setA(vk::ComponentSwizzle::eIdentity);
	components.setB(vk::ComponentSwizzle::eIdentity);
	components.setG(vk::ComponentSwizzle::eIdentity);
	components.setR(vk::ComponentSwizzle::eIdentity);

	vk::ImageSubresourceRange subResRange;
	subResRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subResRange.setLayerCount(1);
	subResRange.setLevelCount(1);
	subResRange.setBaseMipLevel(0);
	subResRange.setBaseArrayLayer(0);
	
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.setImage(this->handle);
	viewInfo.setFormat(format);
	viewInfo.setViewType(vk::ImageViewType::e2D);
	viewInfo.setComponents(components);
	viewInfo.setSubresourceRange(subResRange);
	view = device.createImageView(viewInfo);
}

Image::~Image()
{
	if (isMapped)
	{
		device.unmapMemory(memory);
	}
	if (selfManaged)
	{
		device.destroyImage(handle);
		device.freeMemory(memory);
	}
	if (view)
		device.destroyImageView(view);
}

vk::ImageView& Image::getView()
{
	return view;
}

}
