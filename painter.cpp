#include "painter.hpp"

Painter::Painter (const Swapchain& swapchain) :
	swapchain(swapchain),
	context(swapchain.context),
	window(swapchain.window),
	imageWidth(window.size[0]),
	imageHeight(window.size[1]),
	imageSize(swapchain.window.size[0] * swapchain.window.size[1])
{
	initializeCanvas();
	std::cout << "Painter created!" << std::endl;
}

Painter::~Painter ()
{
	context.device.destroyBuffer(imageBuffer);
	context.device.freeMemory(imageBufferMemory);
}

void Painter::prepare()
{
	createBuffer();
	std::cout << "Painter prepared!" << std::endl;
}

void Painter::paint()
{
	writeToCanvas();
	writeCanvasToBuffer();
}

void Painter::createImage()
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

void Painter::createBuffer()
{
	vk::BufferCreateInfo bufferInfo;

	bufferInfo.setSize(
			swapchain.swapchainExtent.width * 
			swapchain.swapchainExtent.height *
			4); //4 channels
	bufferInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
	bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

	imageBuffer = context.device.createBuffer(bufferInfo);

	auto memReqs = context.device.getBufferMemoryRequirements(imageBuffer);

	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setMemoryTypeIndex(10); //discovered by inspection
	allocInfo.setAllocationSize(memReqs.size);
	std::cout << "Mem reqs size:" << memReqs.size << std::endl;
	memReqsSize = memReqs.size;

	imageBufferMemory = context.device.allocateMemory(allocInfo);

	context.device.bindBufferMemory(imageBuffer, imageBufferMemory, 0);
}

void Painter::initializeCanvas()
{
	canvas.resize(imageSize);
}

void Painter::fillCanvas()
{
	int i = 0;
	while (i < canvas.size())
	{
		i++;
		canvas[i] = UINT32_MAX;
	}
	std::cout << "Part of canvas: " << std::endl;
	std::cout << (unsigned char)canvas[55] << std::endl;
}

void Painter::writeToCanvas()
{
	canvas[window.mouseY*imageWidth + window.mouseX] = UINT32_MAX; //should set to white
}

void Painter::writeCanvasToBuffer()
{
	auto memory = context.device.mapMemory(imageBufferMemory, 0, imageSize);
//	memcpy(memory, canvas.data(), imageSize);
	unsigned char* pImgMem = static_cast<unsigned char*>(memory);


	for (int row = 0; row < imageHeight; row++)
	{
		for (int col = 0; col < imageWidth; col++)
		{
			unsigned char rgb = (((row & 0x8) == 0) ^ ((col & 0x8) == 0)) * 255;
			pImgMem[0] = rgb;
			pImgMem[1] = rgb;
			pImgMem[2] = rgb;
			pImgMem[3] = 255;
			pImgMem += 4;
		}
	}
	//flush it to see if it helps?
	vk::MappedMemoryRange memRange;
	memRange.setSize(memReqsSize);
	memRange.setMemory(imageBufferMemory);
	memRange.setOffset(0);
	context.device.flushMappedMemoryRanges(memRange);

	context.device.unmapMemory(imageBufferMemory);
}

void Painter::checkBufferMemReqs(vk::Buffer buffer)
{
	auto memReqs = context.device.getBufferMemoryRequirements(buffer);
	std::cout << "Required memory type bits" << std::endl;
	std::cout << std::bitset<32>(memReqs.memoryTypeBits) << std::endl;
}
