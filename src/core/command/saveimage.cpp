#include "saveimage.hpp"
#include "render/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <application.hpp>
#include <iostream>
#include <lodepng.h>
#include <queue>

namespace sword
{

namespace command
{

void SaveSwapToPng::execute(Application* app)
{
    render::BufferBlock* block = app->renderer.copySwapToHost();
    vk::Extent2D swapSize = app->renderer.getSwapExtent();
    std::vector<unsigned char> pngBuffer;

    auto memPtr = static_cast<unsigned char*>(block->pHostMemory);

    // the swap image stores pixels in BGRA format, so we have to swizzle 
    // the B and R bytes
    int array_size = swapSize.width * swapSize.height;
    for (int i = 0; i < array_size; i++) 
    {
        unsigned char& B = memPtr[i * 4];   
        unsigned char& R = memPtr[i * 4 + 2];
        std::swap(B, R);
    }

    lodepng::encode(
            pngBuffer,
            memPtr,
            swapSize.width,
            swapSize.height);
    std::string path = "output/images/" + filename + ".png";
    lodepng::save_file(pngBuffer, path);
    success();
}

}; // namespace command

}; // namespace sword
