#include <vulkan/vulkan.hpp>
#include "swapchain.hpp"

class Painter
{
public:
	Painter (const Swapchain& swapchain) :
		swapchain(swapchain)	
	{
		width = swapchain.window.size[0];
		height = swapchain.window.size[1];
	}
	virtual ~Painter ();

private:
	const Swapchain& swapchain;
	int width, height;
	vk::Image image;

	void createImage()
	{
		vk::ImageCreateInfo createInfo;
		vk::Extent3D extent;
		extent.setWidth(width);
		extent.setHeight(height);
		extent.setDepth(1);
		createInfo.setImageType(vk::ImageType::e2D);
		createInfo.setExtent(extent);
		createInfo.setMipLevels(1);
		createInfo.setArrayLayers(1);
		createInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
		createInfo.setTiling(vk::ImageTiling::eLinear);
		createInfo.setInitialLayout(vk::ImageLayout::ePreinitialized);

		//unfinished
	}
};
