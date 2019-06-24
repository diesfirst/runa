#ifndef PAINTER_H
#define PAINTER_H

#include <vulkan/vulkan.hpp>
#include "swapchain.hpp"
#include <bitset>

class Painter
{
public:
	Painter (const Swapchain& swapchain) :
		swapchain(swapchain),
		context(swapchain.context),
		window(swapchain.window),
		imageWidth(window.size[0]),
		imageHeight(window.size[1]),
		imageSize(swapchain.window.size[0] * swapchain.window.size[1])
	{
		initializeCanvas();
	}

	virtual ~Painter ()
	{
		context.device.destroyBuffer(imageBuffer);
		context.device.freeMemory(imageBufferMemory);
	}

	void prepare()
	{
		createBuffer();
	}

	void paint()
	{
		writeToCanvas();
		writeCanvasToBuffer();
	}

	vk::Buffer imageBuffer;

	const size_t imageSize;
private:
	const Swapchain& swapchain;
	const Context& context;
	const Window& window;
	const int imageWidth, imageHeight;
	std::vector<uint32_t> canvas;
	vk::Image image;
	vk::DeviceMemory imageBufferMemory;

	void createImage()
	{
		vk::ImageCreateInfo createInfo;
		vk::Extent3D extent;
		extent.setWidth(imageWidth);
		extent.setHeight(imageHeight);
		extent.setDepth(1);
		createInfo.setImageType(vk::ImageType::e2D);
		createInfo.setExtent(extent);
		createInfo.setMipLevels(1);
		createInfo.setArrayLayers(1);
		createInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
		createInfo.setTiling(vk::ImageTiling::eLinear);
		createInfo.setInitialLayout(vk::ImageLayout::ePreinitialized);
		createInfo.setUsage(
				vk::ImageUsageFlagBits::eStorage |
				vk::ImageUsageFlagBits::eTransferSrc);

		//unfinished
	}

	void createBuffer()
	{
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.setSize(imageSize);
		bufferInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc |
				vk::BufferUsageFlagBits::eStorageBuffer);
		bufferInfo.setSharingMode(vk::SharingMode::eExclusive);
		imageBuffer = context.device.createBuffer(bufferInfo);

		auto memReqs = context.device.getBufferMemoryRequirements(imageBuffer);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.setMemoryTypeIndex(9); //discovered by inspection
		allocInfo.setAllocationSize(memReqs.size);

		imageBufferMemory = context.device.allocateMemory(allocInfo);

		context.device.bindBufferMemory(imageBuffer, imageBufferMemory, 0);
	}

	void initializeCanvas()
	{
		canvas.resize(imageSize);
	}

	void writeToCanvas()
	{
		canvas[window.mouseY*imageWidth + window.mouseX] = UINT32_MAX; //should set to white
	}

	void writeCanvasToBuffer()
	{
		auto memory = context.device.mapMemory(imageBufferMemory, 0, imageSize);
		memcpy(memory, canvas.data(), imageSize);
		context.device.unmapMemory(imageBufferMemory);
	}

	void checkBufferMemReqs(vk::Buffer buffer)
	{
		auto memReqs = context.device.getBufferMemoryRequirements(buffer);
		std::cout << "Required memory type bits" << std::endl;
		std::cout << std::bitset<32>(memReqs.memoryTypeBits) << std::endl;
	}
};
#endif /* ifndef PAINTER_H */
