#include "swapchain.hpp"

Swapchain::Swapchain(const Context& context, const Window& window) :
	context(context),
	window(window)
{
	createSurface();
	queueFamilyIndex = context.pickQueueFamilyIndex(surface);
	getSurfaceCapabilities();
	chooseSwapExtent();
	chooseFormat();
	createSwapchain();
	grabImages();
	createImageViews();
	initializePresentInfo();
}

Swapchain::~Swapchain()
{	
	for (auto fence : imageFences) {
		if (fence) {
			context.device.waitForFences(fence, VK_TRUE, UINT64_MAX);
			context.device.destroyFence(fence);
		}
	}
	destroyFramebuffers();
	destroyImageViews();
	context.device.destroyRenderPass(renderPass);
	context.device.destroySwapchainKHR(swapchain);
	context.instance.destroySurfaceKHR(surface);
}

void Swapchain::checkPresentModes()
{
	std::vector<vk::PresentModeKHR> availablePresentModes;
	availablePresentModes = context.physicalDevice.getSurfacePresentModesKHR(surface);
	int numberOfModes = availablePresentModes.size();
	std::cout << "Number of present modes: " << numberOfModes << std::endl;
	for (int i = 0; i < numberOfModes; ++i)
	{
		const auto mode = availablePresentModes[i];
		if (mode == vk::PresentModeKHR::eFifo) 
		{
			std::cout << "Fifo mode available at index " << 
				i << std::endl;
		}
		if (mode == vk::PresentModeKHR::eMailbox) 
		{
			std::cout << "Mailbox mode at index" <<
			        i << std::endl;	
		}
	}
}

void Swapchain::createSurface()
{
	vk::XcbSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.connection  = window.connection;
	surfaceCreateInfo.window = window.window;
	surface = context.instance.createXcbSurfaceKHR(surfaceCreateInfo);
}

void Swapchain::chooseFormat()
{
	std::vector<vk::SurfaceFormatKHR> surfaceFormats =
		context.physicalDevice.getSurfaceFormatsKHR(surface);
	colorFormat = vk::Format::eB8G8R8A8Unorm; //availabe
	colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear; //available
}

void Swapchain::getSurfaceCapabilities()
{
	surfCaps = context.physicalDevice.getSurfaceCapabilitiesKHR(surface);
}

void Swapchain::chooseSwapExtent()
{
	if (surfCaps.currentExtent == -1) 
	{
		swapchainExtent.width = window.size[0];
		swapchainExtent.height = window.size[1];
	}
	else
	{
		swapchainExtent.width = surfCaps.currentExtent.width;
		swapchainExtent.height = surfCaps.currentExtent.height;
	}
}

void Swapchain::checkImageCounts()
{
	int minImageCount = surfCaps.minImageCount;
	int maxImageCount = surfCaps.maxImageCount;
	std::cout << "Min image count = " << minImageCount << std::endl;
	std::cout << "Max image count = " << maxImageCount << std::endl;
}

void Swapchain::checkCurrentExtent()
{
	int width = surfCaps.currentExtent.width;
	int height = surfCaps.currentExtent.height;
	std::cout << "Surf width: " << 
		width << " height: " << 
		height << std::endl;
}

void Swapchain::checkFormatsAvailable()
{
	std::vector<vk::SurfaceFormatKHR> surfaceFormats =
		context.physicalDevice.getSurfaceFormatsKHR(surface);
	for (const auto format : surfaceFormats) {
		std::cout << "Format: " << (uint32_t)format.format 
			<< " Colorspace: " << (uint32_t)format.colorSpace 
			<< std::endl;
	}
}

void Swapchain::createSwapchain()
{
	vk::SwapchainCreateInfoKHR createInfo;

	createInfo.setSurface(surface);
	createInfo.setImageExtent(swapchainExtent);
	createInfo.setImageFormat(colorFormat);
	createInfo.setImageColorSpace(colorSpace);
	createInfo.setPresentMode(presentMode);
	createInfo.setMinImageCount(surfCaps.minImageCount + 1);
	createInfo.setImageArrayLayers(1); //more than 1 for VR applications
	//we will be drawing directly to the image
	//as opposed to transfering data to it
	createInfo.setImageUsage(
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferDst);
	//this is so that each queue has exclusive 
	//ownership of an image at a time
	createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
	createInfo.setPreTransform(surfCaps.currentTransform);
	createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
	//means we dont care about the color of obscured pixels
	createInfo.setClipped(true); 
	swapchain = context.device.createSwapchainKHR(createInfo);
	std::cout << "Swapchain created!" << std::endl;
	swapchainCreated = true;
}

void Swapchain::grabImages()
{
	if (!swapchainCreated) {
		std::cout << "Attempted to get images"
		       << " from nonexistant swapchain"
	       	       << std::endl;
	}
	images = context.device.getSwapchainImagesKHR(swapchain);
}

void Swapchain::createImageViews()
{
	vk::ImageViewCreateInfo createInfo;
	createInfo.setViewType(vk::ImageViewType::e2D);
	createInfo.setFormat(colorFormat);
	
	vk::ImageSubresourceRange subResRange;
	subResRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subResRange.setLayerCount(1);
	subResRange.setLevelCount(1);

	createInfo.setSubresourceRange(subResRange);

	for (const auto image : images) {
		createInfo.setImage(image);
		imageViews.push_back(context.device.createImageView(createInfo));
	}
}

void Swapchain::destroyImageViews()
{
	for (auto imageView : imageViews) {
		context.device.destroyImageView(imageView);
	}
}


void Swapchain::createColorAttachment(
		std::vector<vk::AttachmentDescription>& attachments)
{
	vk::AttachmentDescription attachment;
	attachment.setFormat(colorFormat);
	attachment.setSamples(vk::SampleCountFlagBits::e1);
	//Sets what to do with data in the attachment
	//before rendering
	attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
	//Sets what we do with the data after rendering
	//We want to show it so we will store it
	attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
	attachment.setInitialLayout(vk::ImageLayout::eUndefined);
	attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	attachments.push_back(attachment);
}

void Swapchain::createRenderPass()
{
	std::vector<vk::AttachmentDescription> attachments;
	std::vector<vk::AttachmentReference> references;
	std::vector<vk::SubpassDescription> subpasses;
	std::vector<vk::SubpassDependency> subpassDependencies;

	vk::AttachmentDescription attachment;
	attachment.setFormat(colorFormat);
	attachment.setSamples(vk::SampleCountFlagBits::e1);
	//Sets what to do with data in the attachment
	//before rendering
	attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
	//Sets what we do with the data after rendering
	//We want to show it so we will store it
	attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
	attachment.setInitialLayout(vk::ImageLayout::eUndefined);
	attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	attachments.push_back(attachment);

	vk::AttachmentReference colorRef;
	colorRef.setAttachment(0); //our first attachment is color
	colorRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	references.push_back(colorRef);

	vk::SubpassDescription subpass;
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpass.setColorAttachmentCount(1);
	subpass.setPColorAttachments(references.data());
	subpasses.push_back(subpass);

	vk::SubpassDependency dependency;
	dependency.setSrcSubpass(
			VK_SUBPASS_EXTERNAL);
	dependency.setSrcAccessMask(
			vk::AccessFlagBits::eColorAttachmentWrite);
	dependency.setSrcStageMask(
			vk::PipelineStageFlagBits::eColorAttachmentOutput);
	dependency.setDstSubpass(
			0);
	dependency.setDstAccessMask(
			vk::AccessFlagBits::eColorAttachmentRead);
	dependency.setDstStageMask(
			vk::PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependencies.push_back(dependency);

	vk::RenderPassCreateInfo createInfo;
	createInfo.setAttachmentCount(attachments.size());
	createInfo.setPAttachments(attachments.data());
	createInfo.setSubpassCount(subpasses.size());
	createInfo.setPSubpasses(subpasses.data());
	createInfo.setDependencyCount(subpassDependencies.size());
	createInfo.setPDependencies(subpassDependencies.data());
	renderPass = context.device.createRenderPass(createInfo);
}

void Swapchain::createFramebuffers()
{
	vk::FramebufferCreateInfo createInfo;
	createInfo.setWidth(swapchainExtent.width);
	createInfo.setHeight(swapchainExtent.height);
	createInfo.setRenderPass(renderPass);
	createInfo.setAttachmentCount(1);
	createInfo.setLayers(1);
	std::array<vk::ImageView, 1> imageViewsTemp;
	createInfo.setPAttachments(imageViewsTemp.data());
	for (const auto image : imageViews) {
		imageViewsTemp[0] = image;
		framebuffers.push_back(
				context.device.createFramebuffer(createInfo));
	}
}

void Swapchain::destroyFramebuffers()
{
	for (auto framebuffer : framebuffers) {
		context.device.destroyFramebuffer(framebuffer);
	}
}

void Swapchain::acquireNextImage(const vk::Semaphore& semaphore)
{
	auto result = context.device.acquireNextImageKHR(
			swapchain,
			UINT64_MAX, //so it will wait forever
			semaphore, //will be signalled when we can present
			vk::Fence()); //empty fence
	if (result.result != vk::Result::eSuccess) {
		std::cerr << 
		"Invalid acquire result: " << 
		vk::to_string(result.result);
		throw std::error_code(result.result);
	}

	currentImage = result.value;
}

void Swapchain::checkSurfaceCapabilities()
{
	const auto suf = surfCaps.supportedUsageFlags;
	std::cout << vk::to_string(suf) << std::endl;
}

void Swapchain::initializeImageFences()
{
	imageFences.resize(images.size());
}

const vk::Fence& Swapchain::getSubmitFence()
{
	auto& curFence = imageFences[currentImage];
	if (curFence) {
		vk::Result fenceResult = vk::Result::eTimeout;
		while (fenceResult == vk::Result::eTimeout) {
			fenceResult = context.device.waitForFences(
				curFence,
				VK_TRUE, //wait for all (just 1 here)
				UINT64_MAX); //wait forever
		}
		context.device.resetFences(curFence);
	}
	else {
		curFence = context.device.createFence(vk::FenceCreateFlags());
	}
	return curFence;
}

void Swapchain::prepareForRender()
{
	createRenderPass();
	createFramebuffers();
	initializeImageFences();
}

void Swapchain::initializePresentInfo()
{
	presentInfo.setSwapchainCount(1);
	presentInfo.setPSwapchains(&swapchain);
	presentInfo.setPImageIndices(&currentImage);
}
