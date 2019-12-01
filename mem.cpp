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

ImageBlock::ImageBlock(const vk::Device& device) :
	device(device)
{
}

ImageBlock::~ImageBlock()
{
//	device.unmapMemory(memory);
	device.destroyImage(image);
	device.freeMemory(memory);
}

MemoryManager::MemoryManager(const vk::Device& device) :
	device(device)
{
}

MemoryManager::~MemoryManager()
{
}


std::unique_ptr<BufferBlock> MemoryManager::createBuffer(uint32_t size, vk::BufferUsageFlagBits usage)
{
	auto block = std::make_unique<BufferBlock>(device);
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
	std::cout << "Mem reqs alignment:" << memReqs.alignment << std::endl;


	block->memory = device.allocateMemory(allocInfo);

	device.bindBufferMemory(block->buffer, block->memory, 0);

	void* res = device.mapMemory(block->memory, 0, size);
	assert(res != (void*)VK_ERROR_MEMORY_MAP_FAILED);
	block->pHostMemory = res;
	block->size = size;

	return block;
}

std::unique_ptr<ImageBlock> MemoryManager::createImage(
		uint32_t width,
		uint32_t height)
{
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	return createImage(width, height, usage);
}
		

std::unique_ptr<ImageBlock> MemoryManager::createImage(
		uint32_t width,
		uint32_t height,
		vk::ImageUsageFlags usage)
{
	auto block = std::make_unique<ImageBlock>(device);

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

	block->image = device.createImage(createInfo);

	auto imgMemReq = device.getImageMemoryRequirements(block->image); 

	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.setAllocationSize(imgMemReq.size);
	memAllocInfo.setMemoryTypeIndex(7); //device local

	block->memory = device.allocateMemory(memAllocInfo);

	device.bindImageMemory(block->image, block->memory, 0);

//	block->pHostMemory = device.mapMemory(block->memory, 0, imgMemReq.size);
//	we wont be mapping the memory right now

	return block;
}

std::vector<BufferBlock*> MemoryManager::createUBOBlocks(
		size_t count, size_t size)
{
	std::vector<BufferBlock*> blockVector;
	for (size_t i = 0; i < count; ++i) 
	{
		auto block = createBuffer(
				size,
				vk::BufferUsageFlagBits::eUniformBuffer);

		blockVector.push_back(block.get()); //must come before move
		uniformBufferBlocks.push_back(std::move(block));
	}
	return blockVector;
}

std::vector<BufferBlock*> MemoryManager::createDynamicUBOBlocks(
		size_t count, size_t size)
{
	std::vector<BufferBlock*> blockVector;
	for (size_t i = 0; i < count; ++i) 
	{
		auto block = createBuffer(
				size,
				vk::BufferUsageFlagBits::eUniformBuffer);

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

BufferBlock* MemoryManager::createStagingBuffer(size_t size)
{
	auto block = createBuffer(
			size, 
			vk::BufferUsageFlagBits::eTransferSrc);
	auto blockPtr = block.get();
	stagingBuffers.push_back(std::move(block));
	return blockPtr;
}

ImageBlock* MemoryManager::createImageBlock(uint32_t width, uint32_t height)
{
	auto block = createImage(width, height);
	auto blockPtr = block.get();
	imageBlocks.push_back(std::move(block));
	return blockPtr;
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
