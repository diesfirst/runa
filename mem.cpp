#include "mem.hpp"
#include <cstring>
#include <iostream>

MemoryManager::MemoryManager(const vk::Device& device) :
	device(device)
{
}

MemoryManager::~MemoryManager()
{
	unmapBuffers();
	destroyBuffers();
}

uint32_t MemoryManager::addBufferBlock()
{
	BufferBlock block;
	BufferBlocks.push_back(block);
	return BufferBlocks.size() - 1;
}

uint32_t MemoryManager::addImageBlock()
{
	ImageBlock block;
	ImageBlocks.push_back(block);
	return ImageBlocks.size() - 1;
}

void MemoryManager::createBuffer(BufferBlock& block, uint32_t size, vk::BufferUsageFlagBits usage)
{
	std::cout << "MM Size: " << size << std::endl;

	vk::BufferCreateInfo bufferInfo;

	bufferInfo.setSize(size);
	bufferInfo.setUsage(usage);
	bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

	block.buffer = device.createBuffer(bufferInfo);

	auto memReqs = device.getBufferMemoryRequirements(block.buffer);

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setMemoryTypeIndex(9); //always host visible for now
	allocInfo.setAllocationSize(memReqs.size);
	std::cout << "Mem reqs size:" << memReqs.size << std::endl;

	block.memory = device.allocateMemory(allocInfo);

	device.bindBufferMemory(block.buffer, block.memory, 0);

	block.pHostMemory = device.mapMemory(block.memory, 0, size);
	block.size = size;
}

uint32_t MemoryManager::createBuffer(uint32_t size, vk::BufferUsageFlagBits usage)
{
	uint32_t index = addBufferBlock();
	BufferBlock& block = BufferBlocks[index];
	std::cout << "MM Size: " << size << std::endl;

	vk::BufferCreateInfo bufferInfo;

	bufferInfo.setSize(size);
	bufferInfo.setUsage(usage);
	bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

	block.buffer = device.createBuffer(bufferInfo);

	auto memReqs = device.getBufferMemoryRequirements(block.buffer);

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setMemoryTypeIndex(9); //always host visible for now
	allocInfo.setAllocationSize(memReqs.size);
	std::cout << "Mem reqs size:" << memReqs.size << std::endl;

	block.memory = device.allocateMemory(allocInfo);

	device.bindBufferMemory(block.buffer, block.memory, 0);

	block.pHostMemory = device.mapMemory(block.memory, 0, size);
	block.size = size;

	return index;
}

uint32_t MemoryManager::createImage(
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		vk::ImageUsageFlagBits usage)
{
	int index = addImageBlock();
	ImageBlock& block = ImageBlocks[index];

	vk::ImageCreateInfo createInfo;
	vk::Extent3D extent;
	extent.setWidth(width);
	extent.setHeight(height);
	extent.setDepth(depth);
	createInfo.setImageType(vk::ImageType::e2D);
	createInfo.setExtent(extent);
	createInfo.setMipLevels(1);
	createInfo.setArrayLayers(1);
	createInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
	createInfo.setTiling(vk::ImageTiling::eLinear);
	createInfo.setInitialLayout(vk::ImageLayout::eUndefined);
	createInfo.setUsage(usage);
	createInfo.setSharingMode(vk::SharingMode::eExclusive);

	block.image = device.createImage(createInfo);

	auto imgMemReq = device.getImageMemoryRequirements(block.image); 

	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.setAllocationSize(imgMemReq.size);
	memAllocInfo.setMemoryTypeIndex(9); //because it worked for the buffer

	block.memory = device.allocateMemory(memAllocInfo);

	device.bindImageMemory(block.image, block.memory, 0);

	block.pHostMemory = device.mapMemory(block.memory, 0, imgMemReq.size);

	return index;
}

void MemoryManager::createUniformBuffers(size_t count, vk::DeviceSize bufferSize)
{
	uniformBufferBlocks.resize(count);
	for (int i = 0; i < count; ++i) 
	{
		createBuffer(
			uniformBufferBlocks[i],
			bufferSize,
			vk::BufferUsageFlagBits::eUniformBuffer);
	}
}

void MemoryManager::unmapBuffers()
{
	for (BufferBlock block : BufferBlocks) {
		device.unmapMemory(block.memory);
	}
	for (BufferBlock block : uniformBufferBlocks) {
		device.unmapMemory(block.memory);
	}
}

void MemoryManager::destroyBuffers()
{
	for (BufferBlock block : BufferBlocks) {
		device.destroyBuffer(block.buffer); device.freeMemory(block.memory);
	}
	for (BufferBlock block : uniformBufferBlocks) {
		device.destroyBuffer(block.buffer); device.freeMemory(block.memory);
	}
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

BufferBlock* MemoryManager::createVertexBlock(size_t size)
{
	int index = createBuffer(
			size, 
			vk::BufferUsageFlagBits::eVertexBuffer);
	BufferBlock* block = &BufferBlocks[index];
	return block;
}

