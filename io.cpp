#include "io.hpp"
#include "lib/lodepng.h"
#include "commander.hpp"
#include "swapchain.hpp"
#include "mem.hpp"
#include <algorithm>

//void io::saveSwapImage(
//		MemoryManager& mm,
//		Commander& commander,
//		Swapchain& swapchain)
//{
//	auto block = mm.createBuffer(
//			swapchain.extent.width *
//			swapchain.extent.height * 4,
//			vk::BufferUsageFlagBits::eTransferDst);
//
//	commander.copyImageToBuffer(
//			swapchain.images[0],
//			block->buffer,
//			swapchain.extent.width,
//			swapchain.extent.height,
//			1);
//
//	//need to swap the red and blue channels 
//	int i = 0;
//	uint8_t* ptr = static_cast<uint8_t*>(block->pHostMemory);
//	while (i < swapchain.extent.width * swapchain.extent.height)
//	{
//		std::swap(ptr[0], ptr[2]);
//		ptr += 4;
//		i++;
//	}
//
//	
//	std::cout << "Copied swap image to buffer" << std::endl;
//	std::vector<u_int8_t> pngBuffer;
//	lodepng::encode(
//			pngBuffer, 
//			static_cast<const unsigned char*>(block->pHostMemory), 
//			swapchain.extent.width, 
//			swapchain.extent.height);
//	std::cout << "Please give the image a name." << std::endl;
//	std::string name = io::requestUserInput();
//	std::string path = "out/" + name + ".png";
//	lodepng::save_file(pngBuffer, path);
//	std::cout << "Image saved to " << path << std::endl;
//}

std::string io::requestUserInput()
{
	std::string word;
	std::cin >> word;
	return word;
}

std::vector<char> io::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file");
	}
		
	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}
