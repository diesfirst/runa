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
		context.device.destroySemaphore(imageAvailableSemaphore);
		context.device.destroySemaphore(renderFinishedSemaphore);
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

void Commander::initializeCommandBuffers(const Swapchain& swapchain, const Painter& painter)
{
	allocateCommandBuffers(swapchain);
	recordCommandBuffers(swapchain, painter);
}

void Commander::allocateCommandBuffers(const Swapchain& swapchain)
{
	commandBuffers.resize(swapchain.framebuffers.size());
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(commandPool);
	allocInfo.setCommandBufferCount(commandBuffers.size());
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
	commandBuffers = context.device.allocateCommandBuffers(allocInfo);
}

void Commander::recordCommandBuffers(const Swapchain& swapchain, const Painter& painter)
{
	vk::RenderPassBeginInfo renderPassBeginInfo;
	vk::Rect2D area;
	area.setExtent(swapchain.swapchainExtent);
	vk::ClearValue clearValue;
	renderPassBeginInfo.setRenderArea(area);
	renderPassBeginInfo.setRenderPass(swapchain.renderPass);
	renderPassBeginInfo.setPClearValues(&clearValue);
	renderPassBeginInfo.setClearValueCount(1);

	vk::CommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.setFlags(
			vk::CommandBufferUsageFlagBits::
			eSimultaneousUse);

	vk::BufferImageCopy region;
	region.setImageExtent({
			swapchain.swapchainExtent.width,
			swapchain.swapchainExtent.height,
			1});
	region.setImageOffset({0,0,0});
	vk::ImageSubresourceLayers imgSubResrc;
	imgSubResrc.setAspectMask(vk::ImageAspectFlagBits::eColor);
	imgSubResrc.setLayerCount(1);
	region.setImageSubresource(imgSubResrc);
	region.setBufferRowLength(0);
	region.setBufferImageHeight(0);
	region.setBufferOffset(0);

	clearValue.color = clearColors[0];

	for (int i = 0; i < commandBuffers.size(); ++i) {
		renderPassBeginInfo.setFramebuffer(swapchain.framebuffers[i]);

		commandBuffers[i].begin(commandBufferBeginInfo);

		commandBuffers[i].copyBufferToImage(
				painter.imageBuffer, 
				swapchain.images[i],
				vk::ImageLayout::eTransferDstOptimal,
				region);

		commandBuffers[i].beginRenderPass(
				renderPassBeginInfo, 
				vk::SubpassContents::eInline);

		commandBuffers[i].endRenderPass();
		commandBuffers[i].end();
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
	vk::SemaphoreCreateInfo createInfo;
	imageAvailableSemaphore = context.device.createSemaphore(createInfo);
	renderFinishedSemaphore = context.device.createSemaphore(createInfo);
	semaphoresCreated = true;
}

void Commander::renderFrame(Swapchain& swapchain)
{
	uint32_t currentBuffer = swapchain.acquireNextImage(
			imageAvailableSemaphore);

	vk::Fence submitFence = swapchain.getSubmitFence();
	
	vk::SubmitInfo submitInfo;
	vk::PipelineStageFlags flags = 
		vk::PipelineStageFlagBits::eBottomOfPipe;
	submitInfo.setCommandBufferCount(commandBuffers.size());
	submitInfo.setPCommandBuffers(commandBuffers.data());
	submitInfo.setWaitSemaphoreCount(1);
	submitInfo.setPWaitSemaphores(&imageAvailableSemaphore);
	submitInfo.setPWaitDstStageMask(&flags);
	submitInfo.setSignalSemaphoreCount(1);
	submitInfo.setPSignalSemaphores(&renderFinishedSemaphore);

	context.queue.submit(submitInfo, submitFence);
	
	swapchain.presentInfo.setWaitSemaphoreCount(1);
	swapchain.presentInfo.setPWaitSemaphores(
			&renderFinishedSemaphore);

	context.queue.presentKHR(swapchain.presentInfo);
}
 
void Commander::cleanUp()
{
	context.queue.waitIdle();
	context.device.waitIdle();
}
