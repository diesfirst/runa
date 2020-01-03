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

Image::Image(
		const vk::Device& device,
		const vk::Extent3D extent,
		const vk::Format format,
		const vk::ImageUsageFlags usageFlags,
		const vk::ImageLayout initialLayout) :
	device(device),
	extent{extent},
	format{format},
	usageFlags{usageFlags}
{
	vk::ImageCreateInfo createInfo;
	createInfo.setImageType(vk::ImageType::e2D);
	createInfo.setExtent(extent);
	createInfo.setMipLevels(1);
	createInfo.setArrayLayers(1);
	createInfo.setFormat(format);
	createInfo.setTiling(vk::ImageTiling::eOptimal);
	createInfo.setInitialLayout(initialLayout);
	createInfo.setUsage(usageFlags);
	createInfo.setSamples(vk::SampleCountFlagBits::e1);
	createInfo.setSharingMode(vk::SharingMode::eExclusive);
	handle = device.createImage(createInfo);

	auto imgMemReq = device.getImageMemoryRequirements(handle); 

	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.setAllocationSize(imgMemReq.size);
	memAllocInfo.setMemoryTypeIndex(7); //device local

	memory  = device.allocateMemory(memAllocInfo);

	device.bindImageMemory(handle, memory, 0);

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

	vk::SamplerCreateInfo samplerInfo;
	//might want to try nearest at some point
	samplerInfo.setMagFilter(vk::Filter::eLinear);
	samplerInfo.setMinFilter(vk::Filter::eLinear);
	samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
	samplerInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge);
	samplerInfo.setAddressModeV(vk::SamplerAddressMode::eClampToEdge);
	samplerInfo.setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
	samplerInfo.setMipLodBias(0.0);
	samplerInfo.setMaxAnisotropy(1.0);
	samplerInfo.setMinLod(0.0);
	samplerInfo.setMaxLod(1.0);
	samplerInfo.setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
	sampler = device.createSampler(samplerInfo);
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

const vk::Extent2D Image::getExtent2D() const 
{
	return vk::Extent2D{extent.width, extent.height};
}

//Image::Image(Image&& other) :
//	device{other.device},
//	handle{std::move(other.handle)},
//	view{std::move(other.view)},
//	memory{std::move(other.memory)},
//	deviceSize{std::move(other.deviceSize)},
//	extent{std::move(other.extent)},
//	layout{other.layout},
//	usageFlags{other.usageFlags},
//	format{other.format},
//	pHostMemory{other.pHostMemory},
//	isMapped{other.isMapped},
//	selfManaged{other.selfManaged}
//{
//	other.isMapped = false;
//	other.selfManaged = false;
//	other.view = nullptr;
//	other.pHostMemory = nullptr;
//	other.handle = nullptr;
//	other.memory = nullptr;
//}

const vk::ImageView& Image::getView() const
{
	return view;
}

const vk::Sampler& Image::getSampler() const
{
	return sampler;
}

}
