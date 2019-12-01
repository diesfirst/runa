#include "commander.hpp"
#include "swapchain.hpp"
#include "description.hpp"

Commander::Commander(vk::Device& device, vk::Queue& queue, uint32_t queueFamily) :
	device(device),
	queue(queue)
{
	createCommandPool(queueFamily);
	createSyncObjects();
}

Commander::~Commander()
{
	if (semaphoresCreated) {
			for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
					device.destroySemaphore(imageAvailableSemaphores[i]);
					device.destroySemaphore(renderFinishedSemaphores[i]);
					device.destroyFence(inFlightFences[i]);
			}
	}
	if (commandPoolCreated)
		device.destroyCommandPool(commandPool);
}

void Commander::createCommandPool(uint32_t queueFamily)
{
	vk::CommandPoolCreateInfo createInfo;
	createInfo.setQueueFamilyIndex(queueFamily);
	createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	commandPool = device.createCommandPool(createInfo);
	commandPoolCreated = true;
}

void Commander::setQueue(vk::Queue& newQueue)
{
	queue = newQueue;
}

void Commander::allocateCommandBuffers(const uint32_t count)
{
	commandBuffers.resize(count);
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(commandPool);
	allocInfo.setCommandBufferCount(commandBuffers.size());
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
	commandBuffers = device.allocateCommandBuffers(allocInfo);
}

vk::CommandBuffer Commander::beginSingleTimeCommand()
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(commandPool);
	allocInfo.setCommandBufferCount(1);
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);

	auto commandBuffers = 
		device.allocateCommandBuffers(allocInfo);

	vk::CommandBuffer commandBuffer = commandBuffers[0];

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	commandBuffer.begin(beginInfo);

	return commandBuffer;
}

void Commander::endSingleTimeCommand(vk::CommandBuffer commandBuffer)
{
	commandBuffer.end();

	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&commandBuffer);

	queue.submit(submitInfo, {}); //null for a fence

	queue.waitIdle();

	device.freeCommandBuffers(commandPool, commandBuffer);
}

void Commander::transitionImageLayout(
		vk::Image image,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout)
{
	vk::ImageSubresourceRange subResRange;
	subResRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subResRange.setLayerCount(1);
	subResRange.setLevelCount(1);
	subResRange.setBaseMipLevel(0);
	subResRange.setBaseArrayLayer(0);

	vk::ImageMemoryBarrier barrier;
	barrier.setImage(image);
	barrier.setOldLayout(oldLayout);
	barrier.setNewLayout(newLayout);
	barrier.setSrcAccessMask({});
	barrier.setDstAccessMask({});
	//this is kind of specific for the purpose of transitioning
	//the painter image for mapping
	barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	barrier.setSubresourceRange(subResRange);

	vk::CommandBuffer commandBuffer = beginSingleTimeCommand();
	commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eHost,
			{},
			nullptr,
			nullptr,
			barrier);
	endSingleTimeCommand(commandBuffer);
}

void Commander::recordCopyImageToSwapImages(
		const Swapchain& swapchain, 
		vk::Image image)
{
	vk::Rect2D area;
	area.setExtent(swapchain.extent);

	vk::CommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.setFlags(
			vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	vk::ImageSubresourceLayers imgSubResrc;
	imgSubResrc.setAspectMask(vk::ImageAspectFlagBits::eColor);
	imgSubResrc.setLayerCount(1);
	imgSubResrc.setMipLevel(0);
	imgSubResrc.setBaseArrayLayer(0);

	vk::Offset3D offset; //assuming this will be 0

	vk::ImageCopy region;
	region.setExtent({
			swapchain.extent.width,
			swapchain.extent.height,
			1});
	region.setSrcOffset(offset);
	region.setDstOffset(offset);
	region.setSrcSubresource(imgSubResrc);
	region.setDstSubresource(imgSubResrc);

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
		
		barrier.setImage(swapchain.images[i]);

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

		barrier.setImage(image);
		barrier.setOldLayout(vk::ImageLayout::eGeneral);
		barrier.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
		barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);

		commandBuffers[i].pipelineBarrier(
				vk::PipelineStageFlagBits::eHost,
				vk::PipelineStageFlagBits::eTransfer,
				{},
				nullptr,
				nullptr,
				barrier);

		//end image layout transition

		commandBuffers[i].copyImage(
				image,
				vk::ImageLayout::eTransferSrcOptimal,
				swapchain.images[i],
				vk::ImageLayout::eTransferDstOptimal,
				region);
		
		//begin image layout transition
		//

		barrier.setImage(image);
		barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
		barrier.setNewLayout(vk::ImageLayout::eGeneral);
		barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barrier.setDstAccessMask({});

		commandBuffers[i].pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eHost,
			{},
			nullptr,
			nullptr,
			barrier);

		barrier.setImage(swapchain.images[i]);
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
		       << "recorded"	<< std::endl;
	}
}

void Commander::recordCopyBufferToImages(
		vk::Buffer buffer,
		std::vector<vk::Image> images,
		uint32_t width, uint32_t height, uint32_t depth)
{
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
		       << "recorded"	<< std::endl;
	}
}

//note this function assumes image is in the correct layout
void Commander::copyBufferToImage(
		vk::Buffer buffer,
		vk::Image image,
		uint32_t width, uint32_t height,
		uint32_t bufferOffset) 
{
	vk::ImageSubresourceLayers imgSubResLayers;
	imgSubResLayers.setAspectMask(vk::ImageAspectFlagBits::eColor);
	imgSubResLayers.setMipLevel(0);
	imgSubResLayers.setBaseArrayLayer(0);
	imgSubResLayers.setLayerCount(1);

	vk::BufferImageCopy region;
	region.setImageExtent({
			width,
			height,
			1});
	region.setImageOffset({0,0,0});
	region.setBufferOffset(bufferOffset); //in bytes
	region.setBufferRowLength(0); //specify possible padding values between rows
	region.setBufferImageHeight(0);
	region.setImageSubresource(imgSubResLayers);

	auto cmdBuffer = beginSingleTimeCommand();

	cmdBuffer.copyBufferToImage(
			buffer,
			image,
			vk::ImageLayout::eTransferDstOptimal,
			region);

	endSingleTimeCommand(cmdBuffer);
}


void Commander::copyImageToBuffer(
		vk::Image image,
		vk::Buffer buffer,
		uint32_t width, uint32_t height, uint32_t depth)
{
	transitionImageLayout(
			image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferSrcOptimal);

	vk::ImageSubresourceLayers imgSubResLayers;
	imgSubResLayers.setAspectMask(vk::ImageAspectFlagBits::eColor);
	imgSubResLayers.setMipLevel(0);
	imgSubResLayers.setBaseArrayLayer(0);
	imgSubResLayers.setLayerCount(1);

	vk::BufferImageCopy region;
	region.setImageExtent({
			width,
			height,
			depth});
	region.setImageOffset({	0,0,0});
	region.setBufferOffset(0);
	region.setBufferRowLength(0);
	region.setBufferImageHeight(0);
	region.setImageSubresource(imgSubResLayers);

	vk::CommandBuffer cmdBuffer = beginSingleTimeCommand();

	cmdBuffer.copyImageToBuffer(
			image,
			vk::ImageLayout::eTransferSrcOptimal,
			buffer,
			1,
			&region);

	endSingleTimeCommand(cmdBuffer);
}

void Commander::recordDraw(
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

void Commander::createSyncObjects()
{
	vk::SemaphoreCreateInfo semaphoreInfo;
	vk::FenceCreateInfo fenceCreateInfo;

	fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		imageAvailableSemaphores[i] = 
			device.createSemaphore(semaphoreInfo);
		renderFinishedSemaphores[i] = 
			device.createSemaphore(semaphoreInfo);
		inFlightFences[i] = 
			device.createFence(fenceCreateInfo);
	}
	semaphoresCreated = true;
}

void Commander::setSwapchainImagesToPresent(Swapchain& swapchain)
{
	for (auto image : swapchain.images) {
		transitionImageLayout(
				image,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR);
	}
}

uint8_t Commander::renderFrame(Swapchain& swapchain)
{
	device.waitForFences(
			1, 
			&inFlightFences[currentFrame], 
			true, 
			UINT64_MAX);
	device.resetFences(
			1, 
			&inFlightFences[currentFrame]);

	currentIndex = swapchain.acquireNextImage(
      		imageAvailableSemaphores[currentFrame]);

	vk::SubmitInfo submitInfo;
	vk::PipelineStageFlags flags = 
		vk::PipelineStageFlagBits::eColorAttachmentOutput;
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&commandBuffers[currentIndex]);
	submitInfo.setWaitSemaphoreCount(1);
	submitInfo.setPWaitDstStageMask(&flags);
	submitInfo.setSignalSemaphoreCount(1);
	submitInfo.setPWaitSemaphores(
			&imageAvailableSemaphores[currentFrame]);
	submitInfo.setPSignalSemaphores(
			&renderFinishedSemaphores[currentFrame]);
	queue.submit(submitInfo, inFlightFences[currentFrame]);
      
	vk::PresentInfoKHR presentInfo;
	presentInfo.setWaitSemaphoreCount(1);
	presentInfo.setPWaitSemaphores(
     		&imageAvailableSemaphores[currentFrame]);
	presentInfo.setPWaitSemaphores(
	 	&renderFinishedSemaphores[currentFrame]);
	presentInfo.setSwapchainCount(1);
	presentInfo.setPImageIndices(&currentIndex);
	presentInfo.setPSwapchains(&swapchain.swapchain);

	queue.presentKHR(presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return currentFrame;
}

void Commander::presentSwapImage(vk::SwapchainKHR swapchain, uint32_t index)
{
	vk::PresentInfoKHR presentInfo;
	presentInfo.setSwapchainCount(1);
	presentInfo.setPImageIndices(&index);
	presentInfo.setPSwapchains(&swapchain);
}

void Commander::resetCommandBuffers()
{
	for (auto commandBuffer : commandBuffers) {
		commandBuffer.reset({}); //not releasing all resources
	}
}

void Commander::resetCommandBuffer(uint32_t index)
{
	commandBuffers[index].reset({});
}

void Commander::resetCurrentBuffer()
{
	device.waitForFences(
			1, 
			&inFlightFences[currentFrame], 
			true, 
			UINT64_MAX);
	commandBuffers[currentIndex].reset({});
}
 
void Commander::cleanUp()
{
	queue.waitIdle();
	device.waitIdle();
}
