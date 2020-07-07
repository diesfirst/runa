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

template <size_t Size>
CommandPool_t<Size>::CommandPool_t(
        const vk::Device& device, 
        const vk::Queue queue,
        uint32_t queueFamilyIndex, 
        vk::CommandPoolCreateFlags flags) :
    queue{queue}
{
    std::cout << this << " CommandPool constructed" << std::endl;
    vk::CommandPoolCreateInfo info;
    info.setQueueFamilyIndex(queueFamilyIndex);
    info.setFlags(flags);
    handle = device.createCommandPoolUnique(info);

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setCommandPool(*handle);
    allocInfo.setCommandBufferCount(Size); //each pool will have 5 buffers 
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    auto bufferHandles = device.allocateCommandBuffersUnique(allocInfo);

    assert (bufferHandles.size() == Size);

    int i = 0;
    for (auto& handle : bufferHandles) 
    {
        auto buffer = std::make_unique<CommandBuffer>(std::move(handle), device, queue);
        primaryCommandBuffers.at(i) = std::move(buffer);
        i++;
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

template <size_t Size>
CommandBuffer& CommandPool_t<Size>::requestCommandBuffer(uint32_t id, vk::CommandBufferLevel level)
{
    assert(level == vk::CommandBufferLevel::ePrimary && "Only primary command buffers supported");
    assert(activePrimaryCommandBufferCount < primaryCommandBuffers.size());
    if (id >= primaryCommandBuffers.size())
    {
        std::cout << "We here" << '\n';
    }
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

template <size_t Size>
void CommandPool_t<Size>::resetPool()
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

void CommandBuffer::bindVertexBuffer(uint32_t firstBinding, const vk::Buffer* buffer, uint32_t offset)
{
    handle->bindVertexBuffers(firstBinding, {*buffer}, {offset}); 
    //first binding, array of buffers, arrays of offsets
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

void CommandBuffer::copyImageToImage(vk::Image& from, vk::Image& to, vk::ImageCopy region)
{
    handle->copyImage(from, vk::ImageLayout::eTransferSrcOptimal, 
            to, vk::ImageLayout::eTransferDstOptimal, region);
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

    fence.getOwner().resetFences(*fence); //has no effect if fence is already unsignalled
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

template class CommandPool_t<5>;
template class CommandPool_t<2>;

}; // namespace render

}; // namespace sword


