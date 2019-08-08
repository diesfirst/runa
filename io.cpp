#include "io.hpp"
#include "lib/lodepng.h"
#include "commander.hpp"
#include "swapchain.hpp"
#include "mem.hpp"
#include <algorithm>

void saveSwapImage(
		MemoryManager& mm,
		Commander& commander,
		Swapchain& swapchain)
{
	int index = mm.createBuffer(
			swapchain.extent.width *
			swapchain.extent.height * 4,
			vk::BufferUsageFlagBits::eTransferDst);

	commander.copyImageToBuffer(
			swapchain.images[0],
			mm.bufferBlocks[index].buffer,
			swapchain.extent.width,
			swapchain.extent.height,
			1);

	//need to swap the red and blue channels 
	int i = 0;
	uint8_t* ptr = static_cast<uint8_t*>(mm.bufferBlocks[index].pHostMemory);
	while (i < swapchain.extent.width * swapchain.extent.height)
	{
		std::swap(ptr[0], ptr[2]);
		ptr += 4;
		i++;
	}

	
	std::cout << "Copied swap image to buffer" << std::endl;
	std::vector<u_int8_t> pngBuffer;
	lodepng::encode(
			pngBuffer, 
			static_cast<const unsigned char*>(mm.bufferBlocks[index].pHostMemory), 
			swapchain.extent.width, 
			swapchain.extent.height);
	std::cout << "Please give the image a name." << std::endl;
	std::string name = requestUserInput();
	std::string path = "out/" + name + ".png";
	lodepng::save_file(pngBuffer, path);
	std::cout << "Image saved to " << path << std::endl;
}

std::string requestUserInput()
{
	std::string word;
	std::cin >> word;
	return word;
}
