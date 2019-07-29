#include "painter.hpp"
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
	imageWidth(swapchain.extent.width),
	imageHeight(swapchain.extent.height)
{
	imageSize = imageWidth * imageHeight;
	std::cout << "Painter created!" << std::endl;
}

Painter::~Painter ()
{
}

void Painter::prepareForBufferPaint()
{
	int index = aquireBufferBlock(
			swapchain.extent.width *
			swapchain.extent.height * 4);
	commander.recordCopyBufferToImages(
			mm.bufferBlocks[index].buffer, 
			swapchain.images,
			swapchain.extent.width,
			swapchain.extent.height,
			1);
	std::cout << "Painter prepared!" << std::endl;
}

void Painter::prepareForImagePaint()
{
	int index = aquireImageBlock(
			imageWidth,
			imageHeight,
			1);
	commander.transitionImageLayout(
			mm.imageBlocks[index].image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eGeneral);
	commander.recordCopyImageToSwapImages(swapchain, mm.imageBlocks[index].image);
	std::cout << "Painter prepared!" << std::endl;
}

int Painter::aquireBufferBlock(uint32_t size)
{
	int index = mm.createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc);
	pBufferMemory = mm.bufferBlocks[index].pHostMemory;
	return index;
}

int Painter::aquireImageBlock(
		uint32_t width,
		uint32_t height,
		uint32_t depth)
{
	int index = mm.createImage(width, height, depth, vk::ImageUsageFlagBits::eTransferSrc);
	pImageMemory = mm.bufferBlocks[index].pHostMemory;
	return index;
}

void Painter::paintImage(int16_t x, int16_t y)
{
	writeToHostImageMemory(x, y);
}

void Painter::paintBuffer(int16_t x, int16_t y)
{
	writeToHostBufferMemory(x, y);
}

void Painter::fillBuffer(
		uint8_t r,
		uint8_t g,
		uint8_t b,
		uint8_t a)
{
	uint32_t iter = 0;
	uint8_t* pointer = static_cast<uint8_t*>(pBufferMemory);
	while (iter < imageSize)
	{
		pointer[0] = r;
		pointer[1] = g;
		pointer[2] = b;
		pointer[3] = a;
		pointer += 4;
		iter++;
	}
}

void Painter::writeToHostBufferMemory(int16_t x, int16_t y)
{
	static_cast<uint32_t*>(pBufferMemory)[y * imageWidth + x] = UINT32_MAX; //should set to white
}

void Painter::writeToHostImageMemory(int16_t x, int16_t y)
{
	static_cast<uint32_t*>(pImageMemory)[y * 504 + x] = UINT32_MAX; //should set to white
}

void Painter::writeCheckersToHostMemory(float x, float y)
{
	unsigned char* pImgMem = static_cast<unsigned char*>(pBufferMemory);
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
