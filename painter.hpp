#ifndef PAINTER_H
#define PAINTER_H

#include <vulkan/vulkan.hpp>
#include <bitset>
#include <memory>

class Swapchain;
class Commander;
class MemoryManager;

struct Bristle
{
	int16_t offsetX;
	int16_t offsetY;
	float alpha;
};

struct Pixel
{
	float r = 0.0;
	float g = 0.0;
	float b = 0.0;
	float a = 0.0;
};

typedef std::vector<Pixel> Layer;

typedef std::unique_ptr<Layer> LayerPointer;

typedef std::vector<Layer> Stack;

class Painter
{
public:
	Painter(const Swapchain&, Commander&, MemoryManager&);
	virtual ~Painter();

	void prepareForImagePaint();

	void prepareForBufferPaint();

	void paint(int16_t x, int16_t y);

	void fillLayer(Layer& layer, float r, float g, float b, float a);

	void fillBuffer(
			uint8_t r,
			uint8_t g,
			uint8_t b,
			uint8_t a);

	void addNewLayer();

	void overLayers(Layer& layerTop, Layer& layerBottom, Layer& target);

	void circleBrush(float radius);

	void setBrushSize(float r);

	void setCurrentColor(float r, float g, float b);

	size_t imageSize;

	void writeLayerToBuffer(Layer& layer);
	
	void writePixelToBuffer(const Pixel& pixel, const size_t index);

	void writeToLayer(Layer& layer, int16_t x, int16_t y, float a);

	void writeToHostImageMemory(int16_t x,int16_t y);
	
	void writeToHostBufferMemory(int16_t x,int16_t y, uint8_t a);

	void writeCheckersToHostMemory(float x, float y);

	size_t getStackSize();

	void switchToLayer(int index);

	void wipeLayer(Layer& layer);

	void writeCurrentLayerToBuffer();

	void writeForegroundToBuffer();

	void writeBackgroundToBuffer();

	void setAlpha(float a);

	void toggleErase();

private:
	const Swapchain& swapchain;
	Commander& commander;
	MemoryManager& mm;
	int imageWidth, imageHeight;
	void* pBufferMemory;
	void* pImageMemory;
	std::vector<Bristle> currentBrush;
	float R, G, B, A;
	Layer foreground, background;
	int curIndex;
	bool eraseMode = false;
	float curBrushSize;
	Stack stack;

	int aquireBufferBlock(uint32_t size);

	int aquireImageBlock(
		uint32_t width,
		uint32_t height,
		uint32_t depth);

	void setBackground();

	void setForeground();

	float calcAlpha(float val, float radius);

};

#endif
