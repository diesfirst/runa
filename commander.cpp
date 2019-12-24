#include "commander.hpp"
#include <iostream>

CommandPool::CommandPool(
		const vk::Device& device, 
		const vk::Queue& queue,
		uint32_t queueFamilyIndex, 
		vk::CommandPoolCreateFlags flags) :
	device(device),
	queue(queue)
{
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
	device(other.device),
	queue(other.queue),
	handle(other.handle),
	primaryCommandBuffers(std::move(other.primaryCommandBuffers)),
	activePrimaryCommandBufferCount(other.activePrimaryCommandBufferCount)
{
	other.handle = nullptr;
}

CommandBuffer& CommandPool::requestCommandBuffer(vk::CommandBufferLevel level)
{
	if (level == vk::CommandBufferLevel::ePrimary)
	{
		if (activePrimaryCommandBufferCount < primaryCommandBuffers.size())
		{
			return *primaryCommandBuffers.at(activePrimaryCommandBufferCount++);
		}
		primaryCommandBuffers.emplace_back(std::make_unique<CommandBuffer>(*this, level));
		activePrimaryCommandBufferCount++;
		return *primaryCommandBuffers.back();
	}
	else
	{
		throw std::runtime_error("No support for secondary command buffers yet");
	}
}

void CommandPool::resetPool()
{
	device.resetCommandPool(handle, {}); //do not release resources
	activePrimaryCommandBufferCount = 0;
}

//level is primary by default
CommandBuffer::CommandBuffer(CommandPool& pool, vk::CommandBufferLevel level) :
	pool(pool),
	queue(pool.queue),
	device(pool.device)
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(pool.handle);
	allocInfo.setCommandBufferCount(1);
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
	buffers = pool.device.allocateCommandBuffers(allocInfo);
	handle = buffers.at(0);

	vk::SemaphoreCreateInfo semaInfo;
	signalSemaphore = pool.device.createSemaphore(semaInfo);

	vk::FenceCreateInfo fenceInfo;
	fence = pool.device.createFence(fenceInfo);
}

CommandBuffer::~CommandBuffer()
{
	std::cout << "device " << device << std::endl;
	pool.device.waitIdle();
//	if (signalSemaphore)
//		pool.device.destroySemaphore(signalSemaphore);
//	if (fence)
//		pool.device.destroyFence(fence);
//	if (handle)
//		pool.device.freeCommandBuffers(pool.handle, handle);
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) :
	pool{other.pool},
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
	other.buffers.clear();
}

void CommandBuffer::begin()
{
	vk::CommandBufferBeginInfo beginInfo;
//	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
//	shoud not need this to resubmit 
	handle.begin(beginInfo);
	recordingComplete = false;
}

void CommandBuffer::beginRenderPass(vk::RenderPassBeginInfo& info)
{
	handle.beginRenderPass(&info, vk::SubpassContents::eInline);
}

void CommandBuffer::drawVerts(uint32_t vertCount, uint32_t firstVertex)
{
	handle.draw(vertCount, 0, firstVertex, 0);
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
	queue.submit(si, nullptr);
	return signalSemaphore;
}

const bool CommandBuffer::isRecorded() const
{
	return recordingComplete;
}

void CommandBuffer::bindGraphicsPipeline(vk::Pipeline& pipeline)
{
	handle.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
}

Commander::Commander(vk::Device& device, vk::Queue& graphicsQueue, uint32_t queueFamily) :
	device(device),
	graphicsQueue(graphicsQueue),
	primaryQueue(graphicsQueue)
{
//	commandPool = std::make_unique<CommandPool>(device, queueFamily);
//	commandPoolCreated = true;
}

Commander::~Commander()
{
}

void Commander::recordCopyBufferToImages(
		std::vector<vk::CommandBuffer>& commandBuffers,
		vk::Buffer& buffer,
		const std::vector<vk::Image>& images,
		uint32_t width, uint32_t height)
{
	assert (commandBuffers.size() == images.size());
	vk::CommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.setFlags(
			vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	vk::ImageSubresourceLayers imgSubResrc;
	imgSubResrc.setAspectMask(vk::ImageAspectFlagBits::eColor);
	imgSubResrc.setLayerCount(1);
	imgSubResrc.setMipLevel(0);
	imgSubResrc.setBaseArrayLayer(0);

	vk::BufferImageCopy region;
	region.setImageExtent({width, height,1});
	region.setImageOffset({0,0,0});
	region.setImageSubresource(imgSubResrc);
	region.setBufferRowLength(0);
	region.setBufferImageHeight(0);
	region.setBufferOffset(0);

	vk::ImageSubresourceRange subResRange;
	subResRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subResRange.setLayerCount(1);
	subResRange.setLevelCount(1);
	subResRange.setBaseMipLevel(0);
	subResRange.setBaseArrayLayer(0);

	vk::ImageMemoryBarrier barrier;
	barrier.setSubresourceRange(subResRange);
	barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

	for (int i = 0; i < commandBuffers.size(); ++i) 
	{

		//begin image layout transition
		
		barrier.setImage(images[i]);

		barrier.setOldLayout(vk::ImageLayout::eUndefined);
		barrier.setNewLayout(vk::ImageLayout::eTransferDstOptimal);
		barrier.setSrcAccessMask({});
		barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

		commandBuffers[i].begin(commandBufferBeginInfo);

		commandBuffers[i].pipelineBarrier(
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eTransfer,
				{},
				nullptr,
				nullptr,
				barrier);

		//end image layout transition

		commandBuffers[i].copyBufferToImage(
				buffer, 
				images[i],
				vk::ImageLayout::eTransferDstOptimal,
				region);
		
		//begin image layout transition
		
		barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
		barrier.setNewLayout(vk::ImageLayout::ePresentSrcKHR);
		barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barrier.setDstAccessMask(vk::AccessFlagBits::eMemoryRead);

		commandBuffers[i].pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eBottomOfPipe,
			{},
			nullptr,
			nullptr,
			barrier);

		commandBuffers[i].end();
		std::cout << "Command Buffer " << commandBuffers[i]
		       << "recorded in Copy Buffer to Images"	<< std::endl;
	}
}

//note this function assumes image is in the correct layout


void Commander::recordDraw(
		std::vector<vk::CommandBuffer>& commandBuffers,
		vk::RenderPass& renderpass,
	 	std::vector<vk::Framebuffer>& framebuffers,
		vk::Buffer& vertexBuffer,
		vk::Buffer& indexBuffer,
		vk::Pipeline& graphicsPipeline,
		vk::PipelineLayout& pipelineLayout,
		std::vector<vk::DescriptorSet>& descriptorSets,
		uint32_t width, uint32_t height,
		std::vector<DrawableInfo>& drawInfos, uint32_t dynamicAlignment)
{
	assert (commandBuffers.size() == framebuffers.size());
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	vk::ClearColorValue clearColorValue;
	clearColorValue.setFloat32({0.1, 0.1, 0.1, 1.0});
	vk::ClearValue clearValue(clearColorValue);

	vk::RenderPassBeginInfo renderPassInfo;
	renderPassInfo.setRenderPass(renderpass);
	renderPassInfo.setRenderArea({{0, 0}, {width, height}});
	renderPassInfo.setClearValueCount(1);
	renderPassInfo.setPClearValues(&clearValue);

	for (int i = 0; i < framebuffers.size(); ++i) 
	{
		std::cout << "in the framebuffer loops" << i << std::endl;
		renderPassInfo.setFramebuffer(framebuffers[i]);
		commandBuffers[i].begin(beginInfo);
		commandBuffers[i].beginRenderPass(
				renderPassInfo, vk::SubpassContents::eInline);
		commandBuffers[i].bindPipeline(
				vk::PipelineBindPoint::eGraphics, 
				graphicsPipeline);
		//first binding, buffers, offsets
		commandBuffers[i].bindVertexBuffers(0, vertexBuffer, {0});
		//buffer, offset, type
		commandBuffers[i].bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

		for (int j = 0; j < drawInfos.size(); ++j) 
		{
			std::cout << "in the draw loop" << j << std::endl;
			uint32_t dynamicOffset = j * dynamicAlignment;
			commandBuffers[i].bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					pipelineLayout,
					0,
					descriptorSets[i],
					dynamicOffset);
//			commandBuffers[i].draw(3, 1, 0, 0);
			commandBuffers[i].drawIndexed(
					drawInfos[j].numberOfIndices,
					1,
					drawInfos[j].indexBufferOffset,
					drawInfos[j].vertexIndexOffset,
					0);
		}
		commandBuffers[i].endRenderPass();
		commandBuffers[i].end();
	}
}

//uint8_t Commander::renderFrame(const vk::SwapchainKHR* swapchain)
//{
//	device.waitForFences(
//			1, 
//			&inFlightFences[currentFrame], 
//			true, 
//			UINT64_MAX);
//	device.resetFences(
//			1, 
//			&inFlightFences[currentFrame]);
//
//	currentIndex = swapchain.acquireNextImage(
//      		imageAvailableSemaphores[currentFrame]);
//
//	vk::SubmitInfo submitInfo;
//	vk::PipelineStageFlags flags = 
//		vk::PipelineStageFlagBits::eColorAttachmentOutput;
//	submitInfo.setCommandBufferCount(1);
//	submitInfo.setPCommandBuffers(&commandBuffers[currentIndex]);
//	submitInfo.setWaitSemaphoreCount(1);
//	submitInfo.setPWaitDstStageMask(&flags);
//	submitInfo.setSignalSemaphoreCount(1);
//	submitInfo.setPWaitSemaphores(
//			&imageAvailableSemaphores[currentFrame]);
//	submitInfo.setPSignalSemaphores(
//			&renderFinishedSemaphores[currentFrame]);
//	queue.submit(submitInfo, inFlightFences[currentFrame]);
//      
//	vk::PresentInfoKHR presentInfo;
//	presentInfo.setWaitSemaphoreCount(1);
//	presentInfo.setPWaitSemaphores(
//	 	&renderFinishedSemaphores[currentFrame]);
//	presentInfo.setSwapchainCount(1);
//	presentInfo.setPImageIndices(&currentIndex);
//	presentInfo.setPSwapchains(swapchain);
//
//	queue.presentKHR(presentInfo);
//
//	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
//
//	return currentFrame;
//}

void Commander::presentSwapImage(vk::SwapchainKHR& swapchain, uint32_t index)
{
	vk::PresentInfoKHR presentInfo;
	presentInfo.setSwapchainCount(1);
	presentInfo.setPImageIndices(&index);
	presentInfo.setPSwapchains(&swapchain);
}

