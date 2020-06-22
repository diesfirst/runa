#include "transfer.hpp"
#include <types/vktypes.hpp>
#include <application.hpp>
#include <render/attachment.hpp>

namespace sword
{

namespace command
{

void CopyAttachmentToDevice::execute(Application* app)
{
    vk::ImageSubresourceRange isr;
    isr.setAspectMask(vk::ImageAspectFlagBits::eColor);
    isr.setLayerCount(1);
    isr.setLevelCount(1);
    isr.setBaseMipLevel(0);
    isr.setBaseArrayLayer(0);

    auto attachmentPtr = app->renderer.getAttachmentPtr(attachmentName);
    auto& image = attachmentPtr->getImage(0).getImage();
    vk::ImageMemoryBarrier imb;
    imb.setImage(image);
    imb.setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    imb.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
    imb.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
    imb.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
    imb.setSubresourceRange(isr);

    commandBuffer->begin();
    commandBuffer->insertImageMemoryBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            imb);

    vk::BufferImageCopy copyRegion;
    copyRegion.setImageExtent({region.extent.width, region.extent.height, 1});
    copyRegion.setImageOffset({region.offset.x, region.offset.y, 0});
    copyRegion.setBufferOffset(bufferBlock->offset);
    copyRegion.setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1});
    copyRegion.setBufferRowLength(0);
    copyRegion.setBufferImageHeight(0);

    commandBuffer->copyImageToBuffer(image, bufferBlock->buffer->getHandle(), copyRegion);
    commandBuffer->end();
    commandBuffer->submit();
}

}; // namespace command

}; // namespace sword
