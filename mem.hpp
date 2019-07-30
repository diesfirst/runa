#ifndef MEM
#define MEM
#include "context.hpp"

struct bufferBlock
{
	vk::Buffer buffer;
	vk::DeviceMemory memory;
	void* pHostMemory;
	unsigned long size;
};

struct imageBlock
{
	vk::Image image;
	vk::DeviceMemory memory;
	void* pHostMemory;
	uint32_t width, height, depth;
};

class MemoryManager
{
public:
	MemoryManager(const Context& context);
	virtual ~MemoryManager ();

	uint32_t createBuffer(uint32_t size, vk::BufferUsageFlagBits);
	uint32_t createImage(
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			vk::ImageUsageFlagBits);

	std::vector<bufferBlock> bufferBlocks;
	std::vector<imageBlock> imageBlocks;

private:
	const Context& context;
	uint32_t addBufferBlock();
	uint32_t addImageBlock();
	void unmapBuffers();
	void destroyBuffers();
	void getImageSubresourceLayout(vk::Image);
};
#endif