#include <render/command.hpp>
#include <iostream>

namespace sword
{

namespace render
{

void waitOnCommandBuffer(CommandBuffer& buffer, const vk::Device& device)
{
    device.waitForFences(buffer.getFence(), true, UINT64_MAX);
}

CommandPool::CommandPool(
        const vk::Device& device, 
        const vk::Queue queue,
        uint32_t queueFamilyIndex, 
        vk::CommandPoolCreateFlags flags) :
    queue{queue}
{
    static constexpr int poolsize = 10;
    std::cout << this << " CommandPool constructed" << std::endl;
    vk::CommandPoolCreateInfo info;
    info.setQueueFamilyIndex(queueFamilyIndex);
    info.setFlags(flags);
    handle = device.createCommandPoolUnique(info);

    primaryCommandBuffers.reserve(poolsize);

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setCommandPool(*handle);
    allocInfo.setCommandBufferCount(poolsize); //each pool will have 5 buffers 
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    auto bufferHandles = device.allocateCommandBuffersUnique(allocInfo);

    for (auto& handle : bufferHandles) 
    {
        primaryCommandBuffers.emplace_back(
                std::make_unique<CommandBuffer>(std::move(handle), device, queue));
    }
}

//CommandPool::CommandPool(CommandPool&& other) :
//    device{other.device},
//    queue{other.queue},
//    handle{other.handle},
//    primaryCommandBuffers{std::move(other.primaryCommandBuffers)},
//    activePrimaryCommandBufferCount{other.activePrimaryCommandBufferCount}
//{
//    std::cout << this << " CommandPool move constructed" << std::endl;
//    other.handle = nullptr;
//}

CommandBuffer& CommandPool::requestCommandBuffer(uint32_t id, vk::CommandBufferLevel level)
{
    assert(level == vk::CommandBufferLevel::ePrimary && "Only primary command buffers supported");
    assert(activePrimaryCommandBufferCount < primaryCommandBuffers.size());
    assert(id < primaryCommandBuffers.size());
    return *primaryCommandBuffers.at(id);
//    if (activePrimaryCommandBufferCount < primaryCommandBuffers.size())
//    {
//        return *primaryCommandBuffers.at(activePrimaryCommandBufferCount++);
//    }
//    auto buffer = std::make_unique<CommandBuffer>(*this, level);
//    primaryCommandBuffers.push_back(std::move(buffer));
//    activePrimaryCommandBufferCount++;
//    return *primaryCommandBuffers.back();
}

void CommandPool::resetPool()
{
//    device.resetCommandPool(handle, {}); //do not release resources
    handle.getOwner().resetCommandPool(*handle, {});
    activePrimaryCommandBufferCount = 0;
}

//level is primary by default
CommandBuffer::CommandBuffer(
        vk::UniqueCommandBuffer&& buffer, 
        const vk::Device& device, 
        const vk::Queue queue,
        vk::CommandBufferLevel level) :
    queue{queue},
    handle{std::move(buffer)}
{
    vk::SemaphoreCreateInfo semaInfo;
    signalSemaphore = device.createSemaphoreUnique(semaInfo);

    vk::FenceCreateInfo fenceInfo;
    fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); //signalled in default state
    fence = device.createFenceUnique(fenceInfo);
}

//CommandBuffer::CommandBuffer(CommandBuffer&& other) :
//    device{other.device},
//    queue{other.queue},
//    buffers{std::move(other.buffers)},
//    handle{buffers.at(0)},
//    signalSemaphore{other.signalSemaphore},
//    fence{other.fence}
//{
//    other.fence = nullptr;
//    other.signalSemaphore = nullptr;
//    other.handle = nullptr;
//}
//
void CommandBuffer::begin()
{
    assert(handle && "command buffer not initialized");
    vk::CommandBufferBeginInfo beginInfo;
//  beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
//  shoud not need this to resubmit 
    handle->reset({});
    handle->begin(beginInfo);
    recordingComplete = false;
}

void CommandBuffer::beginRenderPass(vk::RenderPassBeginInfo& info)
{
    handle->beginRenderPass(&info, vk::SubpassContents::eInline);
}

void CommandBuffer::bindDescriptorSets(
        const vk::PipelineLayout& layout, 
        const std::vector<vk::DescriptorSet>& sets, 
        const std::vector<uint32_t>& offsets)
{
    handle->bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            layout,
            0, //first set is 0
            sets,
            offsets);
}

void CommandBuffer::drawVerts(uint32_t vertCount, uint32_t firstVertex)
{
    handle->draw(vertCount, 1, firstVertex, 0);
}

void CommandBuffer::insertImageMemoryBarrier(
            vk::PipelineStageFlags srcStageMask,
            vk::PipelineStageFlags dstStageMask,
            vk::ImageMemoryBarrier imb)
{
    handle->pipelineBarrier(
            srcStageMask, dstStageMask, 
            {},
            nullptr,
            nullptr,
            imb);
}

void CommandBuffer::copyImageToBuffer(
        vk::Image& image, vk::Buffer& buffer, vk::BufferImageCopy region)
{
    handle->copyImageToBuffer(
            image, vk::ImageLayout::eTransferSrcOptimal, buffer, region);
}


void CommandBuffer::endRenderPass()
{
    handle->endRenderPass();
}

void CommandBuffer::end()
{
    handle->end();
    recordingComplete = true;
}

vk::Semaphore CommandBuffer::submit(
        vk::Semaphore& waitSemaphore, vk::PipelineStageFlags waitMask)
{
    vk::SubmitInfo si;
    si.setPCommandBuffers(&handle.get());
    si.setPWaitSemaphores(&waitSemaphore);
    si.setPSignalSemaphores(&signalSemaphore.get());
    si.setCommandBufferCount(1);
    si.setWaitSemaphoreCount(1);
    si.setSignalSemaphoreCount(1);
    si.setPWaitDstStageMask(&waitMask);

    //device.resetFences(1, &fence);
    fence.getOwner().resetFences(*fence);
    queue.submit(si, fence.get());
    return *signalSemaphore;
}

void CommandBuffer::submit()
{
    vk::SubmitInfo si;
    si.setPCommandBuffers(&handle.get());
    si.setPWaitSemaphores(nullptr);
    si.setPSignalSemaphores(nullptr);
    si.setCommandBufferCount(1);
    si.setWaitSemaphoreCount(0);
    si.setSignalSemaphoreCount(0);
    si.setPWaitDstStageMask(nullptr);

    fence.getOwner().resetFences(*fence);
    queue.submit(si, fence.get());
}

//should restructure this thing, the caller should have a function that waits 
//on the command buffer
void CommandBuffer::waitForFence() const
{
    fence.getOwner().waitForFences(fence.get(), true, UINT64_MAX);
}

void CommandBuffer::reset()
{
    handle->reset({});
}

bool CommandBuffer::isRecorded() const
{
    return recordingComplete;
}

void CommandBuffer::bindGraphicsPipeline(const vk::Pipeline& pipeline)
{
    handle->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
}

const vk::Fence& CommandBuffer::getFence()
{
    return fence.get();
}

}; // namespace render

}; // namespace sword


