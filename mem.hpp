#ifndef MEM
#define MEM
#include <vulkan/vulkan.hpp>
#include <vector>

namespace mm
{

class Buffer
{
friend class MemoryManager;
public:
	Buffer(const vk::Device& device);
	Buffer(
			const vk::Device&,
			vk::PhysicalDeviceMemoryProperties,
			uint32_t size,
			vk::BufferUsageFlags,
			vk::MemoryPropertyFlags);
	~Buffer();
	Buffer(const Buffer&) = delete;
	Buffer& operator=(Buffer&) = delete;
	Buffer& operator=(Buffer&&) = delete;
	Buffer(Buffer&&) = delete;
	vk::Buffer& getHandle();
	uint8_t* getPHostMem();
	uint8_t* map();
	void unmap();

private:
	const vk::Device& device;
	vk::DeviceMemory memory;
	vk::Buffer handle;
	void* pHostMemory{nullptr};
	unsigned long size;
	bool isMapped = false;
	vk::PhysicalDeviceMemoryProperties memoryProperties;

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags);
};

class Image
{
friend class MemoryManager;
public:
	Image(const vk::Device& device);
	Image(
			const vk::Device& device, 
			const vk::Image, 
			const vk::Extent3D, 
			const vk::Format, 
			const vk::ImageUsageFlags);
	~Image();
	vk::ImageView& getView();
private:
	vk::Image handle;
	vk::ImageView view;
	vk::DeviceMemory memory;
	vk::DeviceSize deviceSize;
	vk::Extent3D extent;
	vk::ImageLayout layout;
	vk::ImageUsageFlags usageFlags;
	vk::Format format;
	void* pHostMemory{nullptr};
	bool isMapped = false;
	bool selfManaged = true;
	const vk::Device& device;
};

class MemoryManager
{
public:
	MemoryManager(const vk::Device& device, const vk::PhysicalDevice&);
	virtual ~MemoryManager ();

	std::unique_ptr<Buffer> requestBuffer(uint32_t size, vk::BufferUsageFlagBits);

	std::unique_ptr<Buffer> createBuffer(
			uint32_t size, vk::BufferUsageFlagBits, vk::MemoryPropertyFlags);

	std::unique_ptr<Image> createImage(
			uint32_t width,
			uint32_t height); //automatic transfer dest and sample usage

	std::unique_ptr<Image> createImage(
			vk::Image image, vk::ImageLayout,
			uint32_t width, uint32_t height);

	std::unique_ptr<Image> createImage(
			uint32_t width,
			uint32_t height,
			vk::ImageUsageFlags,
			vk::MemoryPropertyFlags);

	void createUniformBuffers(size_t count, vk::DeviceSize bufferSize);
	std::unique_ptr<Buffer> requestBuffer() const;
	Buffer* createVertexBlock(size_t size);
	Buffer* createIndexBlock(size_t size);
	Buffer* createStagingBuffer(size_t size);
	Image* createImageBlock(uint32_t width, uint32_t height);
	std::vector<Buffer*> createUBOBlocks(
			size_t count, size_t size);
	std::vector<Buffer*> createDynamicUBOBlocks(
			size_t count, size_t size);
	std::unique_ptr<Buffer> vertexBlock;
	std::shared_ptr<Buffer> indexBlock;

	std::vector<Image> ImageBlocks;

	static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags);

private:
	const vk::Device& device;
	const vk::PhysicalDevice& physicalDevice;
	const vk::PhysicalDeviceMemoryProperties memoryProperties;
	uint32_t addBuffer();
	uint32_t addImage();
	void getImageSubresourceLayout(vk::Image);
	std::vector<std::unique_ptr<Buffer>> uniformBuffers;
	std::vector<std::unique_ptr<Buffer>> dynamicUBOBlocks;
	std::vector<std::unique_ptr<Buffer>> stagingBuffers;
	std::vector<std::unique_ptr<Image>> imageBlocks;
};
}
#endif
