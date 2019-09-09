#ifndef PAINTER_H
#define PAINTER_H

#include <vulkan/vulkan.hpp>
#include <bitset>
#include <memory>

class Viewport;
class Commander;
class MemoryManager;

enum class Blend : size_t 
{
	add, sub, min, over, overPreMul, numModes
};

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

typedef std::vector<Pixel> Pixels;

struct Layer
{
	Pixels pixels;
	Blend blendMode = Blend::over;
	float r,g,b,a;
	float brushSize;
};

typedef std::unique_ptr<Layer> LayerPointer;

typedef std::vector<Layer> Stack;

typedef void (*pBlendFunc)(const Pixel&, const Pixel&, Pixel&);

class Painter
{
public:
	Painter(Commander&, MemoryManager&);
	~Painter();

	void prepareForImagePaint();

	void prepareForBufferPaint(const Viewport&);

	void paint(int16_t x, int16_t y);

	void fillPixels(Pixels&, float r, float g, float b, float a);

	void fillBuffer(
			uint8_t r,
			uint8_t g,
			uint8_t b,
			uint8_t a);

	void addNewLayer();

	void overPixels(Pixels& layerTop, Pixels& layerBottom, Pixels& target);

	void circleBrush();

	void setBrushSize(float r);

	void setCurrentColor(float r, float g, float b);

	size_t imageSize;

	void writePixelsToBuffer(Pixels&);
	
	void writePixelToBuffer(const Pixel&, const size_t index);

	void writeToLayer(Layer& layer, int16_t x, int16_t y, float a);

	void writeToHostImageMemory(int16_t x,int16_t y);
	
	void writeCheckersToHostMemory(float x, float y);

	size_t getStackSize();

	void switchToLayer(int index);

	void wipePixels(Pixels& layer);

	void writeCurrentLayerToBuffer();

	void writeForegroundToBuffer();

	void writeBackgroundToBuffer();

	void setAlpha(float a);

	void toggleErase();

	void resizeCanvas(uint32_t width, uint32_t height);

private:
	Commander& commander;
	MemoryManager& mm;
	int imageWidth, imageHeight;
	void* pBufferMemory;
	void* pImageMemory;
	std::vector<Bristle> currentBrush;
	float R, G, B, A; //default values
	Pixels foreground, background;
	int curIndex;
	bool eraseMode = false;
	Blend blendMode = Blend::overPreMul;
	float defaultBrushSize;
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
