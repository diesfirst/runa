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

	void paint();

	void fillCanvas();

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
	uint32_t memReqsSize;

	void createImage();

	void createBuffer();

	void initializeCanvas();

	void writeToCanvas();

	void writeCanvasToBuffer();

	void checkBufferMemReqs(vk::Buffer buffer);
};
#endif /* ifndef PAINTER_H */
