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

class ImageBlock
{
public:
	ImageBlock(const vk::Device& device);
	~ImageBlock();
	vk::Image image;
	vk::DeviceMemory memory;
	void* pHostMemory;
	uint32_t width, height, depth;
private:
	const vk::Device& device;
};

class MemoryManager
{
public:
	MemoryManager(const vk::Device& device);
	virtual ~MemoryManager ();

	std::unique_ptr<BufferBlock> createBuffer(
			uint32_t size, vk::BufferUsageFlagBits);

	std::unique_ptr<ImageBlock> createImage(
			uint32_t width,
			uint32_t height); //automatic transfer dest and sample usage

	std::unique_ptr<ImageBlock> createImage(
			uint32_t width,
			uint32_t height,
			vk::ImageUsageFlags);

	void createUniformBuffers(size_t count, vk::DeviceSize bufferSize);
	BufferBlock* createVertexBlock(size_t size);
	BufferBlock* createIndexBlock(size_t size);
	BufferBlock* createStagingBuffer(size_t size);
	ImageBlock* createImageBlock(uint32_t width, uint32_t height);
	std::vector<BufferBlock*> createUBOBlocks(
			size_t count, size_t size);
	std::vector<BufferBlock*> createDynamicUBOBlocks(
			size_t count, size_t size);
	std::unique_ptr<BufferBlock> vertexBlock;
	std::shared_ptr<BufferBlock> indexBlock;

	std::vector<ImageBlock> ImageBlocks;

private:
	const vk::Device& device;
	uint32_t addBufferBlock();
	uint32_t addImageBlock();
	void getImageSubresourceLayout(vk::Image);
	std::vector<std::unique_ptr<BufferBlock>> uniformBufferBlocks;
	std::vector<std::unique_ptr<BufferBlock>> dynamicUBOBlocks;
	std::vector<std::unique_ptr<BufferBlock>> stagingBuffers;
	std::vector<std::unique_ptr<ImageBlock>> imageBlocks;
};
#endif
