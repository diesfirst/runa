#include "painter.hpp"
#include "swapchain.hpp"
#include "commander.hpp"
#include "mem.hpp"
#include <cmath>
#include <algorithm>

Timer timer;

float length(int x, int y)
{
	return std::sqrt(x*x + y*y);
}

float calcAlpha(float val, float radius)
{
	float alpha = std::clamp(1.0 - val / radius, 0.0, 1.0);
	return alpha;
}

void over(Pixel a, Pixel b, Pixel o)
{
	float complement = 1.0 - a.a;
	o.r = a.r * a.a + b.r * b.a * complement;
	o.g = a.g * a.a + b.g * b.a * complement;
	o.b = a.b * a.a + b.b * b.a * complement;
	a.a = a.a + b.a * complement;
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
	R = G = B = 1.0;
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
	circleBrush(9.0);
	background.resize(imageSize);
	foreground.resize(imageSize);
	target.resize(imageSize);
	std::cout << "foreground size = " << foreground.size() << std::endl;
	fillLayer(background, 0.3, 0.3, 0.3, 1.0);
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

void Painter::paintForeground(int16_t x, int16_t y)
{
	for (Bristle bristle : currentBrush)
	{
		writeToLayer(foreground, bristle.offsetX + x, bristle.offsetY + y, bristle.alpha);
	}
	overLayers(foreground, background, target);
	writeTargetToBuffer();
}

void Painter::paintLayer(Layer& layer, int16_t x, int16_t y)
{
	for (Bristle bristle : currentBrush)
	{
		writeToLayer(layer, bristle.offsetX + x, bristle.offsetY + y, bristle.alpha);
	}
}

void Painter::paintImage(int16_t x, int16_t y)
{
	writeToHostImageMemory(x, y);
}

void Painter::paintBuffer(int16_t x, int16_t y)
{
	timer.start();
	for (Bristle bristle : currentBrush) {
		writeToHostBufferMemory(bristle.offsetX + x, bristle.offsetY + y, 25);
	}
	timer.end();
}

void Painter::fillLayer(Layer& layer, float r, float g, float b, float a)
{
	for (Pixel pixel : layer) {
		pixel.r = r;
		pixel.g = g;
		pixel.b = b;
		pixel.a = a;
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

void Painter::overLayers(Layer& layerTop, Layer& layerBottom, Layer& target)
{
//	//should check that sizes match
//	if (layerTop.size() != layerBottom.size())
//	{
//		std::cout << "Layer sizes don't match" << std::endl;
//		return;
//	}
	size_t i = 0;
	while (i < imageSize)
	{
		over(layerTop[i], layerBottom[i], target[i]);
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
		float alpha = calcAlpha(y, radius);
		Bristle b{0, y, alpha};
		currentBrush.push_back(b);
		y++;
	}
	y = 0;
	x = -1 * iRadius;
	while (x <= iRadius)
	{
		float alpha = calcAlpha(x, radius);
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
			std::cout << "alpha: " << alpha << std::endl;
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
	//sort the currentBrush
}

void Painter::writeTargetToBuffer()
{
	int i = 0;
	uint8_t* ptr = static_cast<uint8_t*>(pBufferMemory);
	while (i < imageSize)
	{
		ptr[0] = static_cast<uint8_t>(target[i].b * 255);
		ptr[1] = static_cast<uint8_t>(target[i].g * 255);
		ptr[2] = static_cast<uint8_t>(target[i].r * 255);
		ptr[3] = static_cast<uint8_t>(target[i].a * 255);
		ptr += 4;
		i++;
	}
}

void Painter::writeToLayer(Layer& layer, int16_t x, int16_t y, float a)
{
	int16_t index = y * imageWidth + x;
	layer[index].r = R;
	layer[index].g = G;
	layer[index].b = B;
	layer[index].a = a;
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
