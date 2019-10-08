#include "mem.hpp"
#include <cstring>
#include <iostream>

BufferBlock::BufferBlock(const vk::Device& device) :
	device(device)
{
}

BufferBlock::~BufferBlock()
{
	device.unmapMemory(memory);
	device.destroyBuffer(buffer);
	device.freeMemory(memory);
}

MemoryManager::MemoryManager(const vk::Device& device) :
	device(device)
{
}

MemoryManager::~MemoryManager()
{
}

uint32_t MemoryManager::addImageBlock()
{
	ImageBlock block;
	ImageBlocks.push_back(block);
	return ImageBlocks.size() - 1;
}

std::shared_ptr<BufferBlock> MemoryManager::createBuffer(uint32_t size, vk::BufferUsageFlagBits usage)
{
	std::shared_ptr<BufferBlock> block = std::make_shared<BufferBlock>(device);
	std::cout << "MM Size: " << size << std::endl;

	vk::BufferCreateInfo bufferInfo;

	bufferInfo.setSize(size);
	bufferInfo.setUsage(usage);
	bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

	block->buffer = device.createBuffer(bufferInfo);

	auto memReqs = device.getBufferMemoryRequirements(block->buffer);

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setMemoryTypeIndex(9); //always host visible for now
	allocInfo.setAllocationSize(memReqs.size);
	std::cout << "Mem reqs size:" << memReqs.size << std::endl;

	block->memory = device.allocateMemory(allocInfo);

	device.bindBufferMemory(block->buffer, block->memory, 0);

	void* res = device.mapMemory(block->memory, 0, size);
	assert(res != (void*)VK_ERROR_MEMORY_MAP_FAILED);
	block->pHostMemory = res;
	block->size = size;

	return block;
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

std::shared_ptr<BufferBlock>* MemoryManager::createUBOBlocks(
		size_t count, size_t size)
{
	for (size_t i = 0; i < count; ++i) 
	{
		auto block = createBuffer(
				size,
				vk::BufferUsageFlagBits::eUniformBuffer);
		uniformBufferBlocks.push_back(block);
	}
	return uniformBufferBlocks.data();
}

std::shared_ptr<BufferBlock>* MemoryManager::createDynamicUBOBlocks(
		size_t count, size_t size)
{
	for (size_t i = 0; i < count; ++i) 
	{
		auto block = createBuffer(
				size,
				vk::BufferUsageFlagBits::eUniformBuffer);
		dynamicUBOBlocks.push_back(block);
	}
	return dynamicUBOBlocks.data();
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
	vertexBlock = createBuffer(
			size, 
			vk::BufferUsageFlagBits::eVertexBuffer);
	return vertexBlock.get();
}

BufferBlock* MemoryManager::createIndexBlock(size_t size)
{
	indexBlock = createBuffer(
			size, 
			vk::BufferUsageFlagBits::eIndexBuffer);
	return indexBlock.get();
}
