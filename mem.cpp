#include "mem.hpp"
#include <cstring>

MemoryManager::MemoryManager(const Context& context) :
	context(context)
{
}

MemoryManager::~MemoryManager()
{
	unmapBuffers();
	destroyBuffers();
}

uint32_t MemoryManager::addBufferBlock()
{
	bufferBlock block;
	bufferBlocks.push_back(block);
	return bufferBlocks.size() - 1;
}

uint32_t MemoryManager::addImageBlock()
{
	imageBlock block;
	imageBlocks.push_back(block);
	return imageBlocks.size() - 1;
}

uint32_t MemoryManager::createBuffer(uint32_t size, vk::BufferUsageFlagBits usage)
{
	uint32_t index = addBufferBlock();
	bufferBlock& block = bufferBlocks[index];
	std::cout << "MM Size: " << size << std::endl;

	vk::BufferCreateInfo bufferInfo;

	bufferInfo.setSize(size);
	bufferInfo.setUsage(usage);
	bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

	block.buffer = context.device.createBuffer(bufferInfo);

	auto memReqs = context.device.getBufferMemoryRequirements(block.buffer);

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setMemoryTypeIndex(9); //always host visible for now
	allocInfo.setAllocationSize(memReqs.size);
	std::cout << "Mem reqs size:" << memReqs.size << std::endl;

	block.memory = context.device.allocateMemory(allocInfo);

	context.device.bindBufferMemory(block.buffer, block.memory, 0);

	block.pHostMemory = context.device.mapMemory(block.memory, 0, size);
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
	imageBlock& block = imageBlocks[index];

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

	block.image = context.device.createImage(createInfo);

	auto imgMemReq = context.device.getImageMemoryRequirements(block.image); 

	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.setAllocationSize(imgMemReq.size);
	memAllocInfo.setMemoryTypeIndex(9); //because it worked for the buffer

	block.memory = context.device.allocateMemory(memAllocInfo);

	context.device.bindImageMemory(block.image, block.memory, 0);

	block.pHostMemory = context.device.mapMemory(block.memory, 0, imgMemReq.size);

	return index;
}

void MemoryManager::unmapBuffers()
{
	for (bufferBlock block : bufferBlocks) {
		context.device.unmapMemory(block.memory);
	}
}

void MemoryManager::destroyBuffers()
{
	for (bufferBlock block : bufferBlocks) {
		context.device.destroyBuffer(block.buffer); context.device.freeMemory(block.memory);
	}
}

void MemoryManager::getImageSubresourceLayout(vk::Image image)
{
	vk::ImageSubresource subresource;
	subresource.setMipLevel(0);
	subresource.setArrayLayer(0);
	subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
	vk::SubresourceLayout subresLayout = 
		context.device.getImageSubresourceLayout(image, subresource);
	std::cout << "Row pitch: " << subresLayout.rowPitch << std::endl;
}

bufferBlock* MemoryManager::vertexBlock(Geo& geo)
{
	int index = createBuffer(
			geo.points.size() * sizeof(Point), 
			vk::BufferUsageFlagBits::eVertexBuffer);
	bufferBlock* block = &bufferBlocks[index];
	std::memcpy(block->pHostMemory, geo.points.data(), (size_t) (sizeof(Point) * geo.points.size()));
	return block;
}

