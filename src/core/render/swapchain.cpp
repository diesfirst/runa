#include <render/context.hpp>
#include <render/swapchain.hpp>
#include <render/surface/window.hpp>

namespace sword
{

namespace render
{

Swapchain::Swapchain(const Context& context, const Window& window, const uint8_t count) :
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
}

//Swapchain::~Swapchain()
//{	
//	destroyImageViews();
//	context.getDevice().destroySwapchainKHR(swapchain);
//	context.getInstance().destroySurfaceKHR(surface);
//}

void Swapchain::createSurface()
{
	vk::XcbSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.connection  = window.connection;
	surfaceCreateInfo.window = window.window;
	surface = context.getInstance().createXcbSurfaceKHR(surfaceCreateInfo);
}

void Swapchain::setQueueFamilyIndex()
{
	context.pickQueueFamilyIndex(surface);
}

void Swapchain::setSurfaceCapabilities()
{
	surfCaps = context.getPhysicalDevice().getSurfaceCapabilitiesKHR(surface);
}

void Swapchain::setFormat()
{
	//make this call simply to make validation layers happy for now
	context.getPhysicalDevice().getSurfaceFormatsKHR(surface);
	colorFormat = vk::Format::eB8G8R8A8Unorm; //availabe
	colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear; //available
}

void Swapchain::setSwapExtent()
{
	if (surfCaps.currentExtent == -1) //forget what this means
	{
		extent.width = window.size[0];
		extent.height = window.size[1];
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
	context.getPhysicalDevice().getSurfacePresentModesKHR(surface);
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
	auto usageFlags = vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferDst |
			vk::ImageUsageFlagBits::eTransferSrc;
	createInfo.setImageUsage(usageFlags);
	//this is so that each queue has exclusive 
	//ownership of an image at a time
	createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
	createInfo.setPreTransform(surfCaps.currentTransform);
	createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
	//means we dont care about the color of obscured pixels
	createInfo.setClipped(true); 
	swapchain = context.getDevice().createSwapchainKHRUnique(createInfo);
	std::cout << "Swapchain created!" << std::endl;
	swapchainCreated = true;
}

std::vector<vk::Image>& Swapchain::getImages()
{
	return images;
}

vk::Extent2D Swapchain::getExtent2D()
{
	return extent;
}

vk::Extent3D Swapchain::getExtent3D()
{
	return vk::Extent3D(extent.width, extent.height, 1);
}

vk::Format Swapchain::getFormat()
{
	return colorFormat;
}

vk::ImageUsageFlags Swapchain::getUsageFlags()
{
	return usageFlags;
}

void Swapchain::setImages()
{
    assert(swapchain);
	images = context.getDevice().getSwapchainImagesKHR(*swapchain);
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
		imageViews.push_back(context.getDevice().createImageViewUnique(createInfo));
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

const vk::SwapchainKHR& Swapchain::getHandle() const
{
	return *swapchain;
}

uint32_t Swapchain::acquireNextImage(vk::Semaphore semaphore, vk::Fence fence)
{
	auto result = context.getDevice().acquireNextImageKHR(
			*swapchain,
			UINT64_MAX, //so it will wait forever
			//will be signalled when we can do something with this
			semaphore, 
			fence);
	if (result.result != vk::Result::eSuccess) 
	{
		std::cerr << "Invalid acquire result: " << vk::to_string(result.result);
		throw std::error_code(result.result);
	}

	return result.value;
}

}; // namespace render

}; // namespace sword


