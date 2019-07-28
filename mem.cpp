#include "mem.hpp"

MemoryManager::MemoryManager(const Context& context) :
	context(context)
{
}

MemoryManager::~MemoryManager()
{
}

uint32_t MemoryManager::addBufferBlock()
{
	bufferBlock block;
	bufferBlocks.push_back(block);
	return bufferBlocks.size() - 1;
}

uint32_t MemoryManager::createBuffer(uint32_t size)
{
	uint32_t index = addBufferBlock();
	bufferBlock& block = bufferBlocks[index];

	vk::BufferCreateInfo bufferInfo;

	bufferInfo.setSize(size);
	bufferInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
	bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

	block.buffer = context.device.createBuffer(bufferInfo);

	auto memReqs = context.device.getBufferMemoryRequirements(block.buffer);

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setMemoryTypeIndex(9); //always host visible for now
	allocInfo.setAllocationSize(memReqs.size);
	std::cout << "Mem reqs size:" << memReqs.size << std::endl;

	block.memory = context.device.allocateMemory(allocInfo);

	context.device.bindBufferMemory(block.buffer, block.memory, 0);

	block.pHostMemory = context.device.mapMemory(block.memory, 0, memReqs.size);
	return index;
}
