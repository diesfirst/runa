#include "saveimage.hpp"
#include "render/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <application.hpp>
#include <iostream>
#include <lodepng.h>
#include <queue>
#include <util/debug.hpp>

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
    std::string path = "/home/michaelb/dev/sword/output/images/" + fileName + ".png";
    lodepng::save_file(pngBuffer, path);
    app->renderer.popBufferBlock();
    success();
}

void SaveAttachmentToPng::execute(Application* app)
{
    render::BufferBlock* block = 
        app->renderer.copyAttachmentToHost(attachmentName, vk::Rect2D{{x, y}, {width, height}});

    if (!block) 
        return;

    std::vector<unsigned char> pngBuffer;

    auto memPtr = static_cast<unsigned char*>(block->pHostMemory);

    int resX = width - x;
    int resY = height - y;

    lodepng::encode(
            pngBuffer,
            memPtr,
            resX,
            resY);

    std::string path;
    SWD_DEBUG_MSG("fileName: " << fileName)
    if (!fullPath)
        path = "/home/michaelb/dev/sword/output/images/" + fileName + ".png";
    else
        path = fileName;
    SWD_DEBUG_MSG("path: " << path)
    auto result = lodepng::save_file(pngBuffer, path);
    if (result != 0)
    {
        std::cerr << "Error saving file: " << lodepng_error_text(result) << '\n';
        return;
    }
    
    app->renderer.popBufferBlock();
    success();
}

void CopyAttachmentToUndoStack::execute(Application* app)
{
    render::BufferBlock* block = 
        app->renderer.copyAttachmentToHost(attachmentName, vk::Rect2D{{x, y}, {width, height}});
    if (!block) return;

    auto memPtr = static_cast<void*>(block->pHostMemory);

    undoStack->copyTo(memPtr, block->size);

    app->renderer.popBufferBlock();
    success();

}

void CopyAttachmentToUndoStack::reverse(Application* app)
{
}


}; // namespace command

}; // namespace sword
