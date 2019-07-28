#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "context.hpp"

class Window; //forward declaration

class Swapchain
{
public:
	Swapchain (const Context& context, const Window& window);

	virtual ~Swapchain();

	void checkPresentModes();

	void checkFormatsAvailable();

	void createSwapchain();

	void createFramebuffers();

	uint32_t acquireNextImage(const vk::Semaphore&);

	const Window& window;
	const Context& context;
	std::vector<vk::Image> images;
	std::vector<vk::ImageView> imageViews;
	vk::SwapchainKHR swapchain;
	vk::SurfaceCapabilitiesKHR surfCaps;
	vk::SurfaceKHR surface;
	std::vector<vk::Framebuffer> framebuffers;
	vk::Extent2D swapchainExtent;
	uint32_t currentIndex{0};
	int imageCount;

private:
	vk::Format colorFormat;
	vk::ColorSpaceKHR colorSpace;
	vk::PresentModeKHR presentMode;
	bool swapchainCreated = false;

	void setImageCount(int count = 3);

	void setQueueFamilyIndex();
	
	void createSurface();

	void setSurfaceCapabilities();
	
	void setSwapExtent(int width=0, int height=0);

	void setFormat();

	void setImages();

	void setPresentMode();

	void createImageViews();

	void destroyImageViews();

};

#endif /* ifndef SWAPCHAIN_H */
