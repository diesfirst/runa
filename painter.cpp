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
	unMapMemory();
	context.device.destroyBuffer(imageBuffer);
	context.device.freeMemory(imageBufferMemory);
}

void Painter::prepare()
{
	createBuffer();
	mapMemory();
	std::cout << "Painter prepared!" << std::endl;
}

void Painter::paint(int16_t x, int16_t y)
{
	writeToHostMemory(x,y);
//	writeCheckersToHostMemory(x,y);
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
	createInfo.setInitialLayout(vk::ImageLayout::eUndefined);
	createInfo.setUsage(
			vk::ImageUsageFlagBits::eTransferSrc);
	image = context.device.createImage(createInfo);

	auto imgMemReq = context.device.getImageMemoryRequirements(image); 
	auto typeBits = imgMemReq.memoryTypeBits;
	std::cout << "Image memory type bits:" << 
	       std::bitset<32>(typeBits) << std::endl; 
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
	while (i < imageSize) 
	{
		i++;
		canvas[i] = UINT32_MAX;
	}
}

void Painter::writeToCanvas()
{
	canvas[window.mouseY*imageWidth + window.mouseX] = UINT32_MAX; //should set to white
}

void Painter::mapMemory()
{
	pHostImageMemory = context.device.mapMemory(
			imageBufferMemory,
			0,
			imageSize);
	std::cout << "memory mapped" << std::endl;
}

void Painter::writeToHostMemory(int16_t x, int16_t y)
{
	static_cast<uint32_t*>(pHostImageMemory)[y * imageWidth + x] = UINT32_MAX; //should set to white
}

void Painter::writeCheckersToHostMemory(float x, float y)
{
	unsigned char* pImgMem = static_cast<unsigned char*>(pHostImageMemory);
	float r = x / imageWidth;
	float g = y / imageHeight;
	for (int row = 0; row < imageHeight; row++)
	{
		for (int col = 0; col < imageWidth; col++)
		{
			unsigned char rgb = (((row & 0x8) == 0) ^ ((col & 0x8) == 0)) * 255;
			pImgMem[0] = rgb * r;
			pImgMem[1] = rgb * g;
			pImgMem[2] = rgb;
			pImgMem[3] = 255;
			pImgMem += 4;
		}
	}
}

void Painter::unMapMemory()
{
	context.device.unmapMemory(imageBufferMemory);
}

void Painter::writeCanvasToBuffer()
{
	auto memory = context.device.mapMemory(imageBufferMemory, 0, imageSize);
	memcpy(memory, canvas.data(), imageSize * 4);
	context.device.unmapMemory(imageBufferMemory);
}

void Painter::writeCheckersToBuffer()
{
	auto memory = context.device.mapMemory(imageBufferMemory, 0, imageSize);
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
//	//flush it to see if it helps?
//	vk::MappedMemoryRange memRange;
//	memRange.setSize(memReqsSize);
//	memRange.setMemory(imageBufferMemory);
//	memRange.setOffset(0);
//	context.device.flushMappedMemoryRanges(memRange);

	context.device.unmapMemory(imageBufferMemory);
}

void Painter::checkBufferMemReqs(vk::Buffer buffer)
{
	auto memReqs = context.device.getBufferMemoryRequirements(buffer);
	std::cout << "Required memory type bits" << std::endl;
	std::cout << std::bitset<32>(memReqs.memoryTypeBits) << std::endl;
}
