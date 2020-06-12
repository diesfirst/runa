#include <render/command.hpp>
#include <iostream>

namespace sword
{

namespace render
{

CommandPool::CommandPool(
        const vk::Device& device, 
        const vk::Queue queue,
        uint32_t queueFamilyIndex, 
        vk::CommandPoolCreateFlags flags) :
    device{device},
    queue{queue}
{
    std::cout << this << " CommandPool constructed" << std::endl;
    vk::CommandPoolCreateInfo info;
    info.setQueueFamilyIndex(queueFamilyIndex);
    info.setFlags(flags);
    handle = device.createCommandPool(info);
}

CommandPool::~CommandPool()
{
    if (handle)
        device.destroyCommandPool(handle);
}

CommandPool::CommandPool(CommandPool&& other) :
    device{other.device},
    queue{other.queue},
    handle{other.handle},
    primaryCommandBuffers{std::move(other.primaryCommandBuffers)},
    activePrimaryCommandBufferCount{other.activePrimaryCommandBufferCount}
{
    std::cout << this << " CommandPool move constructed" << std::endl;
    other.handle = nullptr;
}

CommandBuffer& CommandPool::requestCommandBuffer(vk::CommandBufferLevel level)
{
    assert(level == vk::CommandBufferLevel::ePrimary && "Only primary command buffers supported");
    if (activePrimaryCommandBufferCount < primaryCommandBuffers.size())
    {
        return *primaryCommandBuffers.at(activePrimaryCommandBufferCount++);
    }
    auto buffer = std::make_unique<CommandBuffer>(*this, level);
    primaryCommandBuffers.push_back(std::move(buffer));
    activePrimaryCommandBufferCount++;
    return *primaryCommandBuffers.back();
}

void CommandPool::resetPool()
{
    device.resetCommandPool(handle, {}); //do not release resources
    activePrimaryCommandBufferCount = 0;
}

//level is primary by default
CommandBuffer::CommandBuffer(CommandPool& pool, vk::CommandBufferLevel level) :
    queue(pool.queue),
    device(pool.device)
{
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setCommandPool(pool.handle);
    allocInfo.setCommandBufferCount(1);
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    buffers = device.allocateCommandBuffers(allocInfo);
    handle = buffers.at(0);

    vk::SemaphoreCreateInfo semaInfo;
    signalSemaphore = device.createSemaphore(semaInfo);

    vk::FenceCreateInfo fenceInfo;
    fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); //signalled in default state
    fence = device.createFence(fenceInfo);
}

CommandBuffer::~CommandBuffer()
{
    if (signalSemaphore)
        device.destroySemaphore(signalSemaphore);
    if (fence)
        device.destroyFence(fence);
//  if (handle)
//      device.freeCommandBuffers(pool.handle, handle);
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) :
    device{other.device},
    queue{other.queue},
    buffers{std::move(other.buffers)},
    handle{buffers.at(0)},
    signalSemaphore{other.signalSemaphore},
    fence{other.fence}
{
    other.fence = nullptr;
    other.signalSemaphore = nullptr;
    other.handle = nullptr;
}

void CommandBuffer::begin()
{
    vk::CommandBufferBeginInfo beginInfo;
//  beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
//  shoud not need this to resubmit 
    handle.begin(beginInfo);
    recordingComplete = false;
}

void CommandBuffer::beginRenderPass(vk::RenderPassBeginInfo& info)
{
    handle.beginRenderPass(&info, vk::SubpassContents::eInline);
}

void CommandBuffer::bindDescriptorSets(
        const vk::PipelineLayout& layout, 
        const std::vector<vk::DescriptorSet>& sets, 
        const std::vector<uint32_t>& offsets)
{
    handle.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            layout,
            0, //first set is 0
            sets,
            offsets);
}

void CommandBuffer::drawVerts(uint32_t vertCount, uint32_t firstVertex)
{
    handle.draw(vertCount, 1, firstVertex, 0);
}

void CommandBuffer::insertImageMemoryBarrier(
            vk::PipelineStageFlags srcStageMask,
            vk::PipelineStageFlags dstStageMask,
            vk::ImageMemoryBarrier imb)
{
    handle.pipelineBarrier(
            srcStageMask, dstStageMask, 
            {},
            nullptr,
            nullptr,
            imb);
}

void CommandBuffer::copyImageToBuffer(
        vk::Image& image, vk::Buffer& buffer, vk::BufferImageCopy region)
{
    handle.copyImageToBuffer(
            image, vk::ImageLayout::eTransferSrcOptimal, buffer, region);
}


void CommandBuffer::endRenderPass()
{
    handle.endRenderPass();
}

void CommandBuffer::end()
{
    handle.end();
    recordingComplete = true;
}

vk::Semaphore CommandBuffer::submit(vk::Semaphore& waitSemaphore, vk::PipelineStageFlags waitMask)
{
    vk::SubmitInfo si;
    si.setPCommandBuffers(&handle);
    si.setPWaitSemaphores(&waitSemaphore);
    si.setPSignalSemaphores(&signalSemaphore);
    si.setCommandBufferCount(1);
    si.setWaitSemaphoreCount(1);
    si.setSignalSemaphoreCount(1);
    si.setPWaitDstStageMask(&waitMask);

    device.resetFences(1, &fence);
    queue.submit(si, fence);
    return signalSemaphore;
}

void CommandBuffer::submit()
{
    vk::SubmitInfo si;
    si.setPCommandBuffers(&handle);
    si.setPWaitSemaphores(nullptr);
    si.setPSignalSemaphores(nullptr);
    si.setCommandBufferCount(1);
    si.setWaitSemaphoreCount(0);
    si.setSignalSemaphoreCount(0);
    si.setPWaitDstStageMask(nullptr);

    device.resetFences(1, &fence);
    queue.submit(si, fence);
}

void CommandBuffer::waitForFence() const
{
    device.waitForFences(1, &fence, true, UINT64_MAX);
}

void CommandBuffer::reset()
{
    handle.reset({});
}

bool CommandBuffer::isRecorded() const
{
    return recordingComplete;
}

void CommandBuffer::bindGraphicsPipeline(const vk::Pipeline& pipeline)
{
    handle.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
}


}; // namespace render

}; // namespace sword


