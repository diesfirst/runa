#ifndef PAINTER_H
#define PAINTER_H

#include <vulkan/vulkan.hpp>
#include "swapchain.hpp"
#include <bitset>

class Painter
{
public:
	Painter (const Swapchain& swapchain);
	virtual ~Painter ();

	void prepare();

	void paint(int16_t x, int16_t y);

	void fillCanvas();

	vk::Buffer imageBuffer;

	const size_t imageSize;
	
	void writeCanvasToBuffer();

	void mapMemory();

	void unMapMemory();

	void writeToHostMemory(int16_t x,int16_t y);

	void writeCheckersToHostMemory(float x, float y);

private:
	const Swapchain& swapchain;
	const Context& context;
	const Window& window;
	const int imageWidth, imageHeight;
	std::vector<uint32_t> canvas;
	vk::Image image;
	vk::DeviceMemory imageBufferMemory;
	void* pHostImageMemory;
	uint32_t memReqsSize;

	void createImage();

	void writeCheckersToBuffer();

	void createBuffer();

	void initializeCanvas();

	void writeToCanvas();

	void checkBufferMemReqs(vk::Buffer buffer);

};
#endif /* ifndef PAINTER_H */
