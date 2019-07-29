#ifndef PAINTER_H
#define PAINTER_H

#include <vulkan/vulkan.hpp>
#include <bitset>

class Swapchain;
class Commander;
class MemoryManager;

class Painter
{
public:
	Painter(const Swapchain&, Commander&, MemoryManager&);
	virtual ~Painter();

	void prepareForImagePaint();

	void prepareForBufferPaint();

	void paintImage(int16_t x, int16_t y);
	
	void paintBuffer(int16_t x, int16_t y);

	void fillBuffer(
			uint8_t r,
			uint8_t g,
			uint8_t b,
			uint8_t a);

	size_t imageSize;

	void writeToHostImageMemory(int16_t x,int16_t y);
	
	void writeToHostBufferMemory(int16_t x,int16_t y);

	void writeCheckersToHostMemory(float x, float y);

private:
	const Swapchain& swapchain;
	Commander& commander;
	MemoryManager& mm;
	int imageWidth, imageHeight;
	void* pBufferMemory;
	void* pImageMemory;

	int aquireBufferBlock(uint32_t size);

	int aquireImageBlock(
		uint32_t width,
		uint32_t height,
		uint32_t depth);

};

#endif
