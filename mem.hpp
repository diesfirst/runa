#ifndef MEM
#define MEM
#include <vulkan/vulkan.hpp>
#include <vector>

struct BufferBlock
{
	vk::Buffer buffer;
	vk::DeviceMemory memory;
	void* pHostMemory;
	unsigned long size;
};

struct ImageBlock
{
	vk::Image image;
	vk::DeviceMemory memory;
	void* pHostMemory;
	uint32_t width, height, depth;
};

class MemoryManager
{
public:
	MemoryManager(const vk::Device& device);
	virtual ~MemoryManager ();

	void createBuffer(BufferBlock&, uint32_t size, vk::BufferUsageFlagBits);
	uint32_t createBuffer(uint32_t size, vk::BufferUsageFlagBits);
	uint32_t createImage(
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			vk::ImageUsageFlagBits);
	void createUniformBuffers(size_t count, vk::DeviceSize bufferSize);

	BufferBlock* createVertexBlock(size_t size);

	std::vector<BufferBlock> BufferBlocks;
	std::vector<ImageBlock> ImageBlocks;
	std::vector<BufferBlock> uniformBufferBlocks;

private:
	const vk::Device& device;
	uint32_t addBufferBlock();
	uint32_t addImageBlock();
	void unmapBuffers();
	void destroyBuffers();
	void getImageSubresourceLayout(vk::Image);
};
#endif
