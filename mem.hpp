#include "context.hpp"

struct bufferBlock
{
	vk::Buffer buffer;
	vk::DeviceMemory memory;
	void* pHostMemory;
	unsigned long size;
};

class MemoryManager
{
public:
	MemoryManager(const Context& context);
	virtual ~MemoryManager ();

	uint32_t createBuffer(uint32_t size);

	std::vector<bufferBlock> bufferBlocks;

private:
	const Context& context;
	uint32_t addBufferBlock();
};
