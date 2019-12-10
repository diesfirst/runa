#include "swapchain.hpp"
#include "window.hpp"

Swapchain::Swapchain(const Context& context, const XWindow& window, const uint8_t count) :
	context(context),
	window(window)
{
	imageCount = count;
	createSurface();
	//this will pass the surface to the context in order to select
	//an appropriate queue. currently the context will just print out
	//queue information, and we manually select the queue in another 
	//function. but this keeps validation layers happy.
	setQueueFamilyIndex(); 
	setSurfaceCapabilities();
	setSwapExtent();
	setFormat();
	setPresentMode();
	createSwapchain();
	setImages();
	createImageViews();
	initSyncObjects();
}

Swapchain::~Swapchain()
{	
	destroyImageViews();
	destroySyncObjects();
	context.device.destroySwapchainKHR(swapchain);
	context.instance.destroySurfaceKHR(surface);
}

void Swapchain::createSurface()
{
	vk::XcbSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.connection  = window.connection;
	surfaceCreateInfo.window = window.window;
	surface = context.instance.createXcbSurfaceKHR(surfaceCreateInfo);
}

void Swapchain::initSyncObjects()
{
	acquisitionSemaphores.resize(imageCount);
	acquisitionFences.resize(imageCount);
	vk::SemaphoreCreateInfo semaphoreInfo;
	vk::FenceCreateInfo fenceInfo;
	for (uint8_t i = 0; i < imageCount; i++) 
	{
		acquisitionSemaphores[i] = context.device.createSemaphore(semaphoreInfo);
		acquisitionFences[i] = context.device.createFence(fenceInfo);
		std::cout << "semaphore" << i << ' ' << acquisitionSemaphores[i] << std::endl;
		std::cout << "fence" << i << ' ' << acquisitionFences[i] << std::endl;
	}
	
}

void Swapchain::setQueueFamilyIndex()
{
	context.pickQueueFamilyIndex(surface);
}

void Swapchain::setSurfaceCapabilities()
{
	surfCaps = context.physicalDevice.getSurfaceCapabilitiesKHR(surface);
}

void Swapchain::setFormat()
{
	//make this call simply to make validation layers happy for now
	context.physicalDevice.getSurfaceFormatsKHR(surface);
	colorFormat = vk::Format::eB8G8R8A8Unorm; //availabe
	colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear; //available
}

void Swapchain::setSwapExtent(int width, int height)
{
	if (surfCaps.currentExtent == -1) //forget what this means
	{
		extent.width = window.size[0];
		extent.height = window.size[1];
	}
	if (width!=0 && height!=0)
	{
	}
	else
	{
		extent.width = surfCaps.currentExtent.width;
		extent.height = surfCaps.currentExtent.height;
	}
}

void Swapchain::setPresentMode()
{
	//called just to make validation layers happy
	context.physicalDevice.getSurfacePresentModesKHR(surface);
	presentMode = vk::PresentModeKHR::eImmediate;
}

void Swapchain::setImageCount(const uint32_t count)
{
	imageCount = count;
}

void Swapchain::createSwapchain()
{
	vk::SwapchainCreateInfoKHR createInfo;

	createInfo.setSurface(surface);
	createInfo.setImageExtent(extent);
	createInfo.setImageFormat(colorFormat);
	createInfo.setImageColorSpace(colorSpace);
	createInfo.setPresentMode(presentMode);
	createInfo.setMinImageCount(imageCount);
	createInfo.setImageArrayLayers(1); //more than 1 for VR applications
	//we will be drawing directly to the image
	//as opposed to transfering data to it
	createInfo.setImageUsage(
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferDst |
			vk::ImageUsageFlagBits::eTransferSrc);
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

void Swapchain::setImages()
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

uint8_t Swapchain::getCurrentIndex() const
{
	return currentIndex;
}

uint8_t Swapchain::getImageCount() const
{
	return imageCount;
}

std::tuple<uint32_t, vk::Semaphore*> Swapchain::acquireNextImageNoFence()
{
	syncIndex = (1 + syncIndex) % imageCount;
	auto result = context.device.acquireNextImageKHR(
			swapchain,
			UINT64_MAX, //so it will wait forever
			//will be signalled when we can do something with this
			acquisitionSemaphores[syncIndex], 
			vk::Fence());
	if (result.result != vk::Result::eSuccess) 
	{
		std::cerr << "Invalid acquire result: " << vk::to_string(result.result);
		throw std::error_code(result.result);
	}

	std::cout << "&acquisitionSemaphores[syncIndex]" 
		<< &acquisitionSemaphores[syncIndex] << std::endl;

	std::cout << "Fence: " << acquisitionFences[syncIndex] << std::endl;

	return std::make_tuple(
			result.value, 
			&acquisitionSemaphores[syncIndex]);
}

std::tuple<uint32_t, vk::Semaphore*, vk::Fence*> Swapchain::acquireNextImage()
{
	syncIndex = (1 + syncIndex) % imageCount;
	auto result = context.device.acquireNextImageKHR(
			swapchain,
			UINT64_MAX, //so it will wait forever
			//will be signalled when we can do something with this
			acquisitionSemaphores[syncIndex], 
			acquisitionFences[syncIndex]);
	if (result.result != vk::Result::eSuccess) 
	{
		std::cerr << "Invalid acquire result: " << vk::to_string(result.result);
		throw std::error_code(result.result);
	}

	std::cout << "&acquisitionSemaphores[syncIndex]" 
		<< &acquisitionSemaphores[syncIndex] << std::endl;

	std::cout << "Fence: " << acquisitionFences[syncIndex] << std::endl;

	return std::make_tuple(
			result.value, 
			&acquisitionSemaphores[syncIndex],
			&acquisitionFences[syncIndex]);
}


void Swapchain::destroyImageViews()
{
	for (auto imageView : imageViews) 
	{
		context.device.destroyImageView(imageView);
	}
}

void Swapchain::destroySyncObjects()
{
	for (uint8_t i = 0; i < imageCount; i++) 
	{
		context.device.destroySemaphore(acquisitionSemaphores[i]);
		context.device.destroyFence(acquisitionFences[i]);
	}
	
}
