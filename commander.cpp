#include "commander.hpp"

Commander::Commander(const Context& context) :
	context(context)
{
	createCommandPool();
	createClearColors();
	createSemaphores();
}

Commander::~Commander()
{
	if (semaphoresCreated) {
			for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
					context.device.destroySemaphore(imageAvailableSemaphores[i]);
					context.device.destroySemaphore(renderFinishedSemaphores[i]);
					context.device.destroyFence(inFlightFences[i]);
			}
	}
	if (commandPoolCreated)
		context.device.destroyCommandPool(commandPool);
}

void Commander::createCommandPool()
{
	vk::CommandPoolCreateInfo createInfo;
	createInfo.setQueueFamilyIndex(context.getGraphicsQueueFamilyIndex());
	commandPool = context.device.createCommandPool(createInfo);
	commandPoolCreated = true;
}

void Commander::allocateCommandBuffersForSwapchain(const Swapchain& swapchain)
{
	commandBuffers.resize(swapchain.framebuffers.size());
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(commandPool);
	allocInfo.setCommandBufferCount(commandBuffers.size());
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
	commandBuffers = context.device.allocateCommandBuffers(allocInfo);
}

vk::CommandBuffer Commander::beginSingleTimeCommand()
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(commandPool);
	allocInfo.setCommandBufferCount(1);
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);

	auto commandBuffers = 
		context.device.allocateCommandBuffers(allocInfo);

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

	context.queue.submit(submitInfo, {}); //null for a fence

	context.queue.waitIdle();

	context.device.freeCommandBuffers(commandPool, commandBuffer);
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
	area.setExtent(swapchain.swapchainExtent);

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
			swapchain.swapchainExtent.width,
			swapchain.swapchainExtent.height,
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

void Commander::recordCopyBufferToSwapImages(
		const Swapchain& swapchain, 
		vk::Buffer buffer)
{
	vk::Rect2D area;
	area.setExtent(swapchain.swapchainExtent);

	vk::ClearValue clearValue;

	vk::RenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.setRenderArea(area);
	renderPassBeginInfo.setRenderPass(swapchain.renderPass);
	renderPassBeginInfo.setPClearValues(&clearValue);
	renderPassBeginInfo.setClearValueCount(1);

	vk::CommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.setFlags(
			vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	vk::ImageSubresourceLayers imgSubResrc;
	imgSubResrc.setAspectMask(vk::ImageAspectFlagBits::eColor);
	imgSubResrc.setLayerCount(1);
	imgSubResrc.setMipLevel(0);
	imgSubResrc.setBaseArrayLayer(0);
vk::BufferImageCopy region;
	region.setImageExtent({
			swapchain.swapchainExtent.width,
			swapchain.swapchainExtent.height,
			1});
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


		//end image layout transition

		commandBuffers[i].copyBufferToImage(
				buffer, 
				swapchain.images[i],
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

void Commander::createClearColors()
{
	vk::ClearColorValue red;
	vk::ClearColorValue blue;
	vk::ClearColorValue white;
	red.setFloat32({0.7, 0.4, 0.2, 1.0});
	blue.setFloat32({0.0, 0.0, 1.0, 1.0});
	white.setFloat32({1.0, 1.0, 1.0, 1.0});
	clearColors.push_back(red);
	clearColors.push_back(white);
	clearColors.push_back(blue);
}

void Commander::createSemaphores()
{
	vk::SemaphoreCreateInfo semaphoreInfo;
	vk::FenceCreateInfo fenceCreateInfo;

	fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		imageAvailableSemaphores[i] = 
			context.device.createSemaphore(semaphoreInfo);
		renderFinishedSemaphores[i] = 
			context.device.createSemaphore(semaphoreInfo);
		inFlightFences[i] = 
			context.device.createFence(fenceCreateInfo);
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

void Commander::renderFrame(Swapchain& swapchain)
{
	context.device.waitForFences(
			1, 
			&inFlightFences[currentFrame], 
			true, 
			UINT64_MAX);
	context.device.resetFences(
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
	context.queue.submit(submitInfo, inFlightFences[currentFrame]);
      
	swapchain.presentInfo.setWaitSemaphoreCount(1);
	swapchain.presentInfo.setPWaitSemaphores(
      		&imageAvailableSemaphores[currentFrame]);
	swapchain.presentInfo.setPWaitSemaphores(
			&renderFinishedSemaphores[currentFrame]);
	swapchain.presentInfo.setSwapchainCount(1);
	swapchain.presentInfo.setPImageIndices(&currentIndex);
	swapchain.presentInfo.setPSwapchains(&swapchain.swapchain);

	context.queue.presentKHR(swapchain.presentInfo);

//	std::cout << "Current index: " << currentIndex << std::endl;
//	std::cout << "Current frame: " << currentFrame << std::endl;
//	std::cout << "True frame: " << trueFrame << std::endl;

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	trueFrame++;
}
 
void Commander::cleanUp()
{
	context.queue.waitIdle();
	context.device.waitIdle();
}
