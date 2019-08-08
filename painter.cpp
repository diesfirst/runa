#include "painter.hpp"
#include "swapchain.hpp"
#include "commander.hpp"
#include "mem.hpp"
#include "util.hpp"
#include <cmath>
#include <algorithm>

Timer paintTimer;

float length(int x, int y)
{
	return std::sqrt(x*x + y*y);
}

float Painter::calcAlpha(float val, float radius)
{
	float alpha = std::clamp(1.0 - val / radius, 0.0, 1.0);
	alpha *= 0.5 * A;
	return alpha;
}

void add(const Pixel& a, const Pixel& b, Pixel& o)
{
	o.r = std::min(a.r * a.a + b.r * b.a, 1.0f);
	o.g = std::min(a.g * a.a + b.g * b.a, 1.0f);
	o.b = std::min(a.b * a.a + b.b * b.a, 1.0f);
	o.a = std::min(a.a + b.a, 1.0f);;
}

void over(const Pixel& a, const Pixel& b, Pixel& o)
{
	float complement = 1.0 - a.a;
	o.r = a.r * a.a + b.r * b.a * complement;
	o.g = a.g * a.a + b.g * b.a * complement;
	o.b = a.b * a.a + b.b * b.a * complement;
	o.a = a.a + b.a * complement;
}

void overPreMul(const Pixel& a, const Pixel& b, Pixel& o)
{
	float complement = 1.0 - a.a;
	o.r = a.r + b.r * complement;
	o.g = a.g + b.g * complement;
	o.b = a.b + b.b * complement;
	o.a = a.a + b.a * complement;
}

bool bristleCompare(const Bristle& a, const Bristle& b)
{
	if (a.offsetY < b.offsetY)
		return true;
	if (a.offsetY == b.offsetY)
	{
		if (a.offsetX < b.offsetX)
			return true;
	}
	return false;
}

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
	B = G = 1.0;
	A = 1.0;
	R = 0.0;
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
	circleBrush(15.0);
	foreground.resize(imageSize);
	background.resize(imageSize);
	addNewLayer();
	fillLayer(stack[curIndex], 0.5, 0.3, 0.2, 1.0);
	writeLayerToBuffer(stack[curIndex]);
	setBackground();
	setForeground();
	std::cout << "Painter prepared!" << std::endl;
	std::cout << "Current layer index: " 
		<< curIndex << std::endl;
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

void Painter::paint(int16_t x, int16_t y)
{
	if (x < 0 || x > imageWidth) return;
	if (y < 0 || y > imageHeight) return;
	paintTimer.start();
	for (Bristle& bristle : currentBrush)
	{
		writeToLayer(
				stack[curIndex], 
				bristle.offsetX + x, 
				bristle.offsetY + y, 
				bristle.alpha);
	}
	paintTimer.end("writeToLayer");
}

void Painter::fillLayer(Layer& layer, float r, float g, float b, float a)
{
	for (int i = 0; i < imageSize; ++i) {
		Pixel p{r, g, b, a};
		layer[i] = p;
	}
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

//this also will set the current index to the 
//new layer
void Painter::addNewLayer()
{
	stack.push_back(Layer(imageSize));
	curIndex = stack.size() - 1;
	setBackground();
	setForeground();
}

void Painter::overLayers(Layer& layerTop, Layer& layerBottom, Layer& target)
{
//	//should check that sizes match
//	if (layerTop.size() != layerBottom.size())
//	{
//		std::cout << "Layer sizes don't match" << std::endl;
//		return;
//	}
	int i = 0;
	while (i < imageSize)
	{
		overPreMul(layerTop[i], layerBottom[i], target[i]);
		i++;
	}
}

void Painter::circleBrush(float radius)
{
	currentBrush.clear();
	int iRadius = std::floor(radius);
	int16_t x = 0;
	int16_t y = -1 * iRadius;
	float dist = 0;
	while (y <= iRadius)
	{
		float alpha = calcAlpha(abs(y), radius);
		Bristle b{0, y, alpha};
		currentBrush.push_back(b);
		y++;
	}
	y = 0;
	x = -1 * iRadius;
	while (x <= iRadius)
	{
		float alpha = calcAlpha(abs(x), radius);
		Bristle b{x, 0, alpha};
		currentBrush.push_back(b);
		x++;
	}
	int16_t negX, negY;
	y = 1;
	while (y <= iRadius)
	{
		x = 1;
		dist = 0;
		while (dist < radius)
		{
			dist = length(x, y);
			float alpha = calcAlpha(dist, radius);
			negX = -1 * x;
			negY = -1 * y;
			Bristle tr{x, y, alpha};
			Bristle tl{negX, y, alpha};
			Bristle br{x, negY, alpha};
			Bristle bl{negX, negY, alpha};
			currentBrush.push_back(tr);
			currentBrush.push_back(tl);
			currentBrush.push_back(br);
			currentBrush.push_back(bl);
			x++;
		}
		y++;
	}
	std::sort(
			currentBrush.begin(), 
			currentBrush.end(), 
			bristleCompare);
}

void Painter::setBrushSize(float r)
{
	curBrushSize = r;
	circleBrush(r);
}

void Painter::setCurrentColor(float r, float g, float b)
{
	R = r;
	G = g;
	B = b;
}

void Painter::writeLayerToBuffer(Layer& layer)
{
	int i = 0;
	uint8_t* ptr = static_cast<uint8_t*>(pBufferMemory);
	while (i < imageSize)
	{
		ptr[0] = static_cast<uint8_t>(layer[i].b * 255);
		ptr[1] = static_cast<uint8_t>(layer[i].g * 255);
		ptr[2] = static_cast<uint8_t>(layer[i].r * 255);
		ptr[3] = static_cast<uint8_t>(layer[i].a * 255);
		ptr += 4;
		i++;
	}
}

void Painter::writePixelToBuffer(const Pixel& pixel, const size_t index)
{
	uint8_t* ptr = static_cast<uint8_t*>(pBufferMemory);
	ptr += 4*index;
	ptr[0] = static_cast<uint8_t>(pixel.b * 255);
	ptr[1] = static_cast<uint8_t>(pixel.g * 255);
	ptr[2] = static_cast<uint8_t>(pixel.r * 255);
	ptr[3] = static_cast<uint8_t>(pixel.a * 255);
}

void Painter::writeToLayer(Layer& layer, int16_t x, int16_t y, float a)
{
	int index = y * imageWidth + x;
	Pixel pixel{R * a, G * a, B * a, a};
	overPreMul(pixel, layer[index], layer[index]);
	overPreMul(foreground[index], layer[index], pixel);
	overPreMul(pixel, background[index], pixel);
	writePixelToBuffer(pixel, index);
}

void Painter::writeToHostBufferMemory(int16_t x, int16_t y, uint8_t a)
{
//	static_cast<uint32_t*>(pBufferMemory)[y * imageWidth + x] = UINT32_MAX; //should set to white
	uint8_t* ptr = static_cast<uint8_t*>(pBufferMemory);
	ptr += 4 * (y * imageWidth + x);
	ptr[0] = R;
	ptr[1] = G;
	ptr[2] = B;
	ptr[3] = a;
}

void Painter::writeToHostImageMemory(int16_t x, int16_t y)
{
	static_cast<uint32_t*>(pImageMemory)[y * 504 + x] = UINT32_MAX; //should set to white
}

void Painter::switchToLayer(int index)
{
	curIndex = index;
	setBackground();
	setForeground();
}

void Painter::setBackground()
{
	if (curIndex > 0)
	{
		int i = curIndex - 1;
		background = stack[i];
		std::cout << "setting background to layer " << i << std::endl;
		assert(background[0].a == stack[i][0].a);
		assert(background[0].r == stack[i][0].r);
		while (i > 0)
		{
			std::cout << "set bg while loop entered" << std::endl;
			overLayers(background, stack[i-1], background);
			i--;
		}
	}
	else 
	{
		std::cout << "bg wiped" << std::endl;
		wipeLayer(background);
	}
}

void Painter::setForeground()
{
	size_t stacksize = getStackSize();
	std::cout << "from set fg: stacksize - 1: " << stacksize - 1	<< std::endl;
	if (curIndex < (stacksize - 1)) //there are layers above it
	{
		int i = stacksize - 1;
		foreground = stack[i];
		std::cout << "setting foreground to layer " << i << std::endl;
		while (i - 1  > curIndex)
		{
			overLayers(foreground, stack[i-1], foreground);
			i--;
		}
	}
	else
	{
		wipeLayer(foreground);
		std::cout << "fg wiped" << std::endl;
	}
}

void Painter::wipeLayer(Layer& layer)
{
	fillLayer(layer, 0.0, 0.0, 0.0, 0.0);
}

size_t Painter::getStackSize()
{
	return stack.size();
}

void Painter::writeCurrentLayerToBuffer()
{
	writeLayerToBuffer(stack[curIndex]);
}

void Painter::writeForegroundToBuffer()
{
	writeLayerToBuffer(foreground);
}

void Painter::writeBackgroundToBuffer()
{
	writeLayerToBuffer(background);
}

void Painter::setAlpha(float a)
{
	A = a;
	circleBrush(curBrushSize);
	eraseMode = false;
}

void Painter::toggleErase()
{
	if (eraseMode)
	{
		A = 1.0;
		std::cout << "Eraser off" << std::endl;
		eraseMode = false;
		return;
	}
	else
	{
		A = 0.0;
		std::cout << "Erase on" << std::endl;
		eraseMode = true;
	}
	circleBrush(curBrushSize);
}
