#include "transfer.hpp"
#include <types/vktypes.hpp>
#include <application.hpp>
#include <render/attachment.hpp>

namespace sword
{

namespace command
{

void CopyAttachmentToImage::execute(Application* app)
{
    vk::ImageSubresourceRange isr;
    isr.setAspectMask(vk::ImageAspectFlagBits::eColor);
    isr.setLayerCount(1);
    isr.setLevelCount(1);
    isr.setBaseMipLevel(0);
    isr.setBaseArrayLayer(0);

    auto attachmentPtr = app->renderer.getAttachmentPtr(attachmentName);

    auto& attachmentImage = attachmentPtr->getImage(0).getImage();
    auto& destinationImage = image->getImage();

    vk::ImageMemoryBarrier attachmentImageBarrier;
    attachmentImageBarrier.setImage(attachmentImage);
    attachmentImageBarrier.setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    attachmentImageBarrier.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
    attachmentImageBarrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
    attachmentImageBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
    attachmentImageBarrier.setSubresourceRange(isr);

    vk::ImageMemoryBarrier destinationImageBarrier;
    destinationImageBarrier.setImage(destinationImage);
    destinationImageBarrier.setOldLayout(vk::ImageLayout::eUndefined);
    destinationImageBarrier.setNewLayout(vk::ImageLayout::eTransferDstOptimal);
    destinationImageBarrier.setSrcAccessMask({}); //dont know what should go here
    destinationImageBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
    destinationImageBarrier.setSubresourceRange(isr);

    commandBuffer->begin();

    commandBuffer->insertImageMemoryBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            attachmentImageBarrier);

    commandBuffer->insertImageMemoryBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            destinationImageBarrier);

    vk::ImageCopy copyRegion;
    copyRegion.setSrcOffset({region.offset.x, region.offset.y, 1});
    copyRegion.setDstOffset({region.offset.x, region.offset.y, 1});
    copyRegion.setExtent({region.extent.width, region.extent.height, 1});
    copyRegion.setSrcSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1});
    copyRegion.setDstSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1});

    commandBuffer->copyImageToImage(attachmentImage, destinationImage, copyRegion);

    commandBuffer->end();

    commandBuffer->submit();

    success();
}

void CopyImageToAttachment::execute(Application* app)
{
    vk::ImageSubresourceRange isr;
    isr.setAspectMask(vk::ImageAspectFlagBits::eColor);
    isr.setLayerCount(1);
    isr.setLevelCount(1);
    isr.setBaseMipLevel(0);
    isr.setBaseArrayLayer(0);

    auto attachmentPtr = app->renderer.getAttachmentPtr(attachmentName);

    auto& attachmentImage = attachmentPtr->getImage(0).getImage();
    auto& sourceImage = image->getImage();

    vk::ImageMemoryBarrier attachmentImageBarrier;
    attachmentImageBarrier.setImage(attachmentImage);
    attachmentImageBarrier.setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    attachmentImageBarrier.setNewLayout(vk::ImageLayout::eTransferDstOptimal);
    attachmentImageBarrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
    attachmentImageBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
    attachmentImageBarrier.setSubresourceRange(isr);

    vk::ImageMemoryBarrier sourceImageBarrier;
    sourceImageBarrier.setImage(sourceImage);
    sourceImageBarrier.setOldLayout(vk::ImageLayout::eUndefined);
    sourceImageBarrier.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
    sourceImageBarrier.setSrcAccessMask({}); //dont know what should go here
    sourceImageBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
    sourceImageBarrier.setSubresourceRange(isr);

    commandBuffer->begin();

    commandBuffer->insertImageMemoryBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            attachmentImageBarrier);

    commandBuffer->insertImageMemoryBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            sourceImageBarrier);

    vk::ImageCopy copyRegion;
    copyRegion.setSrcOffset({region.offset.x, region.offset.y, 1});
    copyRegion.setDstOffset({region.offset.x, region.offset.y, 1});
    copyRegion.setExtent({region.extent.width, region.extent.height, 1});
    copyRegion.setSrcSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1});
    copyRegion.setDstSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1});

    commandBuffer->copyImageToImage(sourceImage, attachmentImage, copyRegion);

    commandBuffer->end();

    commandBuffer->submit();

    success();
}

}; // namespace command

}; // namespace sword
