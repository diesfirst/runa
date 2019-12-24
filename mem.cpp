#include "mem.hpp"
#include <cstring>
#include <iostream>

namespace mm
{

Buffer::Buffer(const vk::Device& device) :
	device(device)
{
}

Buffer::~Buffer()
{
	device.unmapMemory(memory);
	device.destroyBuffer(handle);
	device.freeMemory(memory);
}

vk::Buffer& Buffer::getVkBuffer()
{
	return handle;
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

MemoryManager::MemoryManager(const vk::Device& device, const vk::PhysicalDevice& physDev) :
	device(device),
	physicalDevice(physDev),
	memoryProperties(physDev.getMemoryProperties())
{
}

MemoryManager::~MemoryManager()
{
}



std::unique_ptr<Buffer> MemoryManager::createBuffer(
		uint32_t size, 
		vk::BufferUsageFlagBits usage,
		vk::MemoryPropertyFlags typeFlags)
{
	std::cout << "Creating buffer!" << std::endl;
	auto buffer = std::make_unique<Buffer>(device);
	std::cout << "MM Size: " << size << std::endl;

	vk::BufferCreateInfo bufferInfo;

	bufferInfo.setSize(size);
	bufferInfo.setUsage(usage);
	bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

	buffer->handle = device.createBuffer(bufferInfo);

	auto memReqs = device.getBufferMemoryRequirements(buffer->handle);
	uint32_t memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, typeFlags); 
	std::cout << "Memory Type index: " << memoryTypeIndex << std::endl;

	vk::MemoryAllocateInfo allocInfo;
	//allocInfo.setMemoryTypeIndex(9); //always host visible for now
	allocInfo.setAllocationSize(memReqs.size);
	allocInfo.setMemoryTypeIndex(memoryTypeIndex);
	std::cout << "Mem reqs size:" << memReqs.size << std::endl;
	std::cout << "Mem reqs alignment:" << memReqs.alignment << std::endl;

	buffer->memory = device.allocateMemory(allocInfo);

	device.bindBufferMemory(buffer->handle, buffer->memory, 0);

	return buffer;
}

std::unique_ptr<Image> MemoryManager::createImage(
		uint32_t width,
		uint32_t height)
{
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc;
	return createImage(width, height, usage, vk::MemoryPropertyFlagBits::eDeviceLocal);
}

//mainly intended for wrapping up swapchain images
std::unique_ptr<Image> MemoryManager::createImage(
		vk::Image handle, vk::ImageLayout layout, 
		uint32_t width, uint32_t height)
{
	auto image = std::make_unique<Image>(device);
	image->handle = handle;
	image->layout = layout;
	return image;
}

std::unique_ptr<Image> MemoryManager::createImage(
		uint32_t width,
		uint32_t height,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags typeFlags)
{
	auto image = std::make_unique<Image>(device);

	vk::ImageCreateInfo createInfo;
	vk::Extent3D extent;
	extent.setWidth(width);
	extent.setHeight(height);
	extent.setDepth(1);
	createInfo.setImageType(vk::ImageType::e2D);
	createInfo.setExtent(extent);
	createInfo.setMipLevels(1);
	createInfo.setArrayLayers(1);
	createInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
	createInfo.setTiling(vk::ImageTiling::eOptimal);
	createInfo.setInitialLayout(vk::ImageLayout::eUndefined);
	createInfo.setUsage(usage);
	createInfo.setSamples(vk::SampleCountFlagBits::e1);
	createInfo.setSharingMode(vk::SharingMode::eExclusive);

	image->handle = device.createImage(createInfo);

	auto memReqs = device.getImageMemoryRequirements(image->handle); 
	uint32_t memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, typeFlags); 

	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.setAllocationSize(memReqs.size);
	memAllocInfo.setMemoryTypeIndex(memoryTypeIndex); //was setting it to 7

	image->memory = device.allocateMemory(memAllocInfo);

	device.bindImageMemory(image->handle, image->memory, 0);

//	image->pHostMemory = device.mapMemory(image->memory, 0, imgMemReq.size);
//	we wont be mapping the memory right now

	return image;
}

std::vector<Buffer*> MemoryManager::createUBOBlocks(
		size_t count, size_t size)
{
	std::vector<Buffer*> blockVector;
	for (size_t i = 0; i < count; ++i) 
	{
		auto block = createBuffer(
				size,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostCoherent |
				vk::MemoryPropertyFlagBits::eHostVisible);

		blockVector.push_back(block.get()); //must come before move
		uniformBuffers.push_back(std::move(block));
	}
	return blockVector;
}

std::vector<Buffer*> MemoryManager::createDynamicUBOBlocks(
		size_t count, size_t size)
{
	std::vector<Buffer*> blockVector;
	for (size_t i = 0; i < count; ++i) 
	{
		auto block = createBuffer(
				size,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostCoherent | 
				vk::MemoryPropertyFlagBits::eHostVisible);

		blockVector.push_back(block.get()); //must come before move
		dynamicUBOBlocks.push_back(std::move(block));
	}
	return blockVector;
}

void MemoryManager::getImageSubresourceLayout(vk::Image image)
{
	vk::ImageSubresource subresource;
	subresource.setMipLevel(0);
	subresource.setArrayLayer(0);
	subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
	vk::SubresourceLayout subresLayout = 
		device.getImageSubresourceLayout(image, subresource);
	std::cout << "Row pitch: " << subresLayout.rowPitch << std::endl;
}

Buffer* MemoryManager::createStagingBuffer(size_t size)
{
	auto block = createBuffer(
			size, 
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostCoherent |
			vk::MemoryPropertyFlagBits::eHostVisible);
	auto blockPtr = block.get();
	stagingBuffers.push_back(std::move(block));
	return blockPtr;
}

Buffer* MemoryManager::createVertexBlock(size_t size)
{
	vertexBlock = createBuffer(
			size, 
			vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	return vertexBlock.get();
}

Buffer* MemoryManager::createIndexBlock(size_t size)
{
	indexBlock = createBuffer(
			size, 
			vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	return indexBlock.get();
}

uint32_t MemoryManager::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
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


}


