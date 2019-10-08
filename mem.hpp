#ifndef MEM
#define MEM
#include <vulkan/vulkan.hpp>
#include <vector>

class BufferBlock
{
public:
	BufferBlock(const vk::Device& device);
	~BufferBlock();
	vk::Buffer buffer;
	vk::DeviceMemory memory;
	void* pHostMemory;
	unsigned long size;
private:
	const vk::Device& device;
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

	std::shared_ptr<BufferBlock> createBuffer(
			uint32_t size, vk::BufferUsageFlagBits);
	uint32_t createImage(
			uint32_t width,
			uint32_t height,
			uint32_t depth,
			vk::ImageUsageFlagBits);
	void createUniformBuffers(size_t count, vk::DeviceSize bufferSize);
	BufferBlock* createVertexBlock(size_t size);
	BufferBlock* createIndexBlock(size_t size);
	std::shared_ptr<BufferBlock>* createUBOBlocks(
			size_t count, size_t size);
	std::shared_ptr<BufferBlock>* createDynamicUBOBlocks(
			size_t count, size_t size);
	std::shared_ptr<BufferBlock> vertexBlock;
	std::shared_ptr<BufferBlock> indexBlock;

	std::vector<ImageBlock> ImageBlocks;
	std::vector<std::shared_ptr<BufferBlock>> uniformBufferBlocks;
	std::vector<std::shared_ptr<BufferBlock>> dynamicUBOBlocks;

private:
	const vk::Device& device;
	uint32_t addBufferBlock();
	uint32_t addImageBlock();
	void getImageSubresourceLayout(vk::Image);
};
#endif
