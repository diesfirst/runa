#ifndef PAINTER_H
#define PAINTER_H

#include <vulkan/vulkan.hpp>
#include <bitset>

class Swapchain;
class Commander;
class Context;
class Window;
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

	const size_t imageSize;
	
	void writeCanvasToBuffer();

	void mapBufferMemory();

	void mapImageMemory();

	void unmapBufferMemory();

	void unmapImageMemory();

	void writeToHostImageMemory(int16_t x,int16_t y);
	
	void writeToHostBufferMemory(int16_t x,int16_t y);

	void writeCheckersToHostMemory(float x, float y);

private:
	const Swapchain& swapchain;
	const Context& context;
	const Window& window;
	Commander& commander;
	MemoryManager& mm;
	const int imageWidth, imageHeight;
	std::vector<uint32_t> canvas;
	vk::Image image;
	vk::Buffer imageBuffer;
	vk::DeviceMemory imageMemory;
	vk::DeviceMemory bufferMemory;
	void* pHostBufferMemory;
	void* pHostImageMemory;
	uint32_t memReqsSize;
	bool imageBufferCreated = false;
	bool imageCreated = false;
	
	void getImageSubresourceLayout();

	void createImage();

	void writeCheckersToBuffer();

	void createBuffer();

	void checkBufferMemReqs(vk::Buffer buffer);
};

#endif
