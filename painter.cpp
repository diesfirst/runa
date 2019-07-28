#include "painter.hpp"
#include "window.hpp"
#include "swapchain.hpp"
#include "commander.hpp"
#include "mem.hpp"

Painter::Painter (
		const Swapchain& swapchain, 
		Commander& commander,
		MemoryManager& mm) :
	swapchain(swapchain),
	commander(commander),
	mm(mm),
	context(swapchain.context),
	window(swapchain.window),
	imageWidth(window.size[0]),
	imageHeight(window.size[1]),
	imageSize(swapchain.window.size[0] * swapchain.window.size[1])
{
	std::cout << "Painter created!" << std::endl;
}

Painter::~Painter ()
{
	if (imageBufferCreated)
	{
		unmapBufferMemory();
		context.device.destroyBuffer(imageBuffer);
		context.device.freeMemory(bufferMemory);
	}
	if (imageCreated)
	{
		unmapImageMemory();
		context.device.destroyImage(image);
		context.device.freeMemory(imageMemory);
	}
}

void Painter::prepareForBufferPaint()
{
	createBuffer();
	mapBufferMemory();
	commander.recordCopyBufferToImages(
			imageBuffer, 
			swapchain.images,
			swapchain.swapchainExtent.width,
			swapchain.swapchainExtent.height,
			1);
	std::cout << "Painter prepared!" << std::endl;
}

void Painter::paintBuffer(int16_t x, int16_t y)
{
	writeToHostBufferMemory(x, y);
}

void Painter::prepareForImagePaint()
{
	createImage();
	commander.transitionImageLayout(
			image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eGeneral);
	mapImageMemory();
	commander.recordCopyImageToSwapImages(swapchain, image);
	std::cout << "Painter prepared!" << std::endl;
	getImageSubresourceLayout();
}

void Painter::paintImage(int16_t x, int16_t y)
{
	writeToHostImageMemory(x, y);
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
	createInfo.setUsage(vk::ImageUsageFlagBits::eTransferSrc);
	createInfo.setSharingMode(vk::SharingMode::eExclusive);

	image = context.device.createImage(createInfo);

	auto imgMemReq = context.device.getImageMemoryRequirements(image); 
	auto typeBits = imgMemReq.memoryTypeBits;
	std::cout << "Image memory type bits:" << 
	       std::bitset<32>(typeBits) << std::endl; 
	//not going to check what bits are required

	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.setAllocationSize(imgMemReq.size);
	memAllocInfo.setMemoryTypeIndex(9); //because it worked for the buffer

	imageMemory = context.device.allocateMemory(memAllocInfo);

	context.device.bindImageMemory(image, imageMemory, 0);

	imageCreated = true;
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
	allocInfo.setMemoryTypeIndex(9); //discovered by inspection
	allocInfo.setAllocationSize(memReqs.size);
	std::cout << "Mem reqs size:" << memReqs.size << std::endl;
	memReqsSize = memReqs.size;

	bufferMemory = context.device.allocateMemory(allocInfo);

	context.device.bindBufferMemory(imageBuffer, bufferMemory, 0);

	imageBufferCreated = true;
}

void Painter::mapImageMemory()
{
	pHostImageMemory = context.device.mapMemory(
			imageMemory,
			0,
			imageSize);
	std::cout << "image memory mapped" << std::endl;
}

void Painter::mapBufferMemory()
{
	pHostBufferMemory = context.device.mapMemory(
			bufferMemory,
			0,
			imageSize);
	std::cout << "memory mapped" << std::endl;
}

void Painter::unmapBufferMemory()
{
	context.device.unmapMemory(bufferMemory);
}

void Painter::unmapImageMemory()
{
	context.device.unmapMemory(imageMemory);
}

void Painter::writeToHostBufferMemory(int16_t x, int16_t y)
{
	static_cast<uint32_t*>(pHostBufferMemory)[y * imageWidth + x] = UINT32_MAX; //should set to white
}

void Painter::writeToHostImageMemory(int16_t x, int16_t y)
{
	static_cast<uint32_t*>(pHostImageMemory)[y * 504 + x] = UINT32_MAX; //should set to white
}

void Painter::writeCheckersToHostMemory(float x, float y)
{
	unsigned char* pImgMem = static_cast<unsigned char*>(pHostBufferMemory);
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

void Painter::checkBufferMemReqs(vk::Buffer buffer)
{
	auto memReqs = context.device.getBufferMemoryRequirements(buffer);
	std::cout << "Required memory type bits" << std::endl;
	std::cout << std::bitset<32>(memReqs.memoryTypeBits) << std::endl;
}

void Painter::getImageSubresourceLayout()
{
	vk::ImageSubresource subresource;
	subresource.setMipLevel(0);
	subresource.setArrayLayer(0);
	subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
	vk::SubresourceLayout subresLayout = 
		context.device.getImageSubresourceLayout(image, subresource);
	std::cout << "Row pitch: " << subresLayout.rowPitch << std::endl;
}
