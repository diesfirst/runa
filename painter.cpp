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
	alpha *= 0.5;
	return alpha;
}

void add(const Pixel& a, const Pixel& b, Pixel& o)
{
	o.r = std::min(a.r * a.a + b.r * b.a, 1.0f);
	o.g = std::min(a.g * a.a + b.g * b.a, 1.0f);
	o.b = std::min(a.b * a.a + b.b * b.a, 1.0f);
	o.a = std::min(a.a + b.a, 1.0f);;
}

void sub(const Pixel& a, const Pixel& b, Pixel& o)
{
	o.r = std::max(b.r - a.r, 0.0f);
	o.g = std::max(b.g - a.g, 0.0f);
	o.b = std::max(b.b - a.b, 0.0f);
	o.a = std::max(b.a - a.a, 0.0f);;
}

void min(const Pixel& a, const Pixel& b, Pixel& o)
{
	o.r = std::min(a.r, b.r);
	o.g = std::min(a.g, b.g);
	o.b = std::min(a.b, b.b);
	o.a = std::min(a.a, b.a);
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

std::array<pBlendFunc, static_cast<size_t>(Blend::numModes)> blendFunctions =
{
	&add,
	&sub,
	&min,
	&over,
	&overPreMul
};

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
	defaultBrushSize = 15.0;
	R = G = B = .9; //default color
	A = 1.0; //default alpha
	foreground.resize(imageSize);
	background.resize(imageSize);
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
	addNewLayer();
	fillPixels(stack[curIndex].pixels, 0.5, 0.3, 0.2, 1.0);
	writePixelsToBuffer(stack[curIndex].pixels);
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

void Painter::fillPixels(Pixels& pixels, float r, float g, float b, float a)
{
	for (int i = 0; i < imageSize; ++i) {
		Pixel p{r, g, b, a};
		pixels[i] = p;
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
	stack.push_back(Layer{
			Pixels(imageSize),
			Blend::over,
			R,
			G,
			B,
			A,
			defaultBrushSize});
	curIndex = stack.size() - 1;
	circleBrush();
	setBackground();
	setForeground();
}

void Painter::overPixels(Pixels& pixelsTop, Pixels& pixelsBottom, Pixels& target)
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
		overPreMul(pixelsTop[i], pixelsBottom[i], target[i]);
		i++;
	}
}

void Painter::circleBrush()
{
	currentBrush.clear();
	float radius = stack[curIndex].brushSize;
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
	stack[curIndex].brushSize = r;
	circleBrush();
}

void Painter::setCurrentColor(float r, float g, float b)
{
	stack[curIndex].r = r;
	stack[curIndex].g = g;
	stack[curIndex].b = b;
}

void Painter::writePixelsToBuffer(Pixels& pixels)
{
	int i = 0;
	uint8_t* ptr = static_cast<uint8_t*>(pBufferMemory);
	while (i < imageSize)
	{
		ptr[0] = static_cast<uint8_t>(pixels[i].b * 255);
		ptr[1] = static_cast<uint8_t>(pixels[i].g * 255);
		ptr[2] = static_cast<uint8_t>(pixels[i].r * 255);
		ptr[3] = static_cast<uint8_t>(pixels[i].a * 255);
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
	if (x > imageWidth || x < 0)
		return;
	if (y > imageHeight || y < 0)
		return;
	int index = y * imageWidth + x;
//dont know why this extra check is necessary
//but it crashes without it
	if (index > imageSize) 
		return;
	a *= layer.a;
	Pixel pixel{layer.r * a, layer.g * a, layer.b * a, a};
	blendFunctions[static_cast<size_t>(blendMode)](
			pixel, 
			layer.pixels[index], 
			layer.pixels[index]);
	overPreMul(foreground[index], layer.pixels[index], pixel);
	overPreMul(pixel, background[index], pixel);
	writePixelToBuffer(pixel, index);
}

void Painter::writeToHostImageMemory(int16_t x, int16_t y)
{
	static_cast<uint32_t*>(pImageMemory)[y * 504 + x] = UINT32_MAX; //should set to white
}

void Painter::switchToLayer(int index)
{
	curIndex = index;
	circleBrush();
	setBackground();
	setForeground();
}

void Painter::setBackground()
{
	if (curIndex > 0)
	{
		int i = curIndex - 1;
		background = stack[i].pixels;
		std::cout << "setting background to layer " << i << std::endl;
		assert(background[0].a == stack[i].pixels[0].a);
		assert(background[0].r == stack[i].pixels[0].r);
		while (i > 0)
		{
			std::cout << "set bg while loop entered" << std::endl;
			overPixels(background, stack[i-1].pixels, background);
			i--;
		}
	}
	else 
	{
		std::cout << "bg wiped" << std::endl;
		wipePixels(background);
	}
}

void Painter::setForeground()
{
	size_t stacksize = getStackSize();
	std::cout << "from set fg: stacksize - 1: " << stacksize - 1	<< std::endl;
	if (curIndex < (stacksize - 1)) //there are layers above it
	{
		int i = stacksize - 1;
		foreground = stack[i].pixels;
		std::cout << "setting foreground to layer " << i << std::endl;
		while (i - 1  > curIndex)
		{
			overPixels(foreground, stack[i-1].pixels, foreground);
			i--;
		}
	}
	else
	{
		wipePixels(foreground);
		std::cout << "fg wiped" << std::endl;
	}
}

void Painter::wipePixels(Pixels& pixels)
{
	fillPixels(pixels, 0.0, 0.0, 0.0, 0.0);
}

size_t Painter::getStackSize()
{
	return stack.size();
}

void Painter::writeCurrentLayerToBuffer()
{
	writePixelsToBuffer(stack[curIndex].pixels);
}

void Painter::writeForegroundToBuffer()
{
	writePixelsToBuffer(foreground);
}

void Painter::writeBackgroundToBuffer()
{
	writePixelsToBuffer(background);
}

void Painter::setAlpha(float a)
{
	stack[curIndex].a = a;
}

void Painter::toggleErase()
{
	if (eraseMode)
	{
		std::cout << "Eraser off" << std::endl;
		eraseMode = false;
		blendMode = Blend::overPreMul;
	}
	else
	{
		std::cout << "Erase on" << std::endl;
		eraseMode = true;
		blendMode = Blend::sub;
	}
}
