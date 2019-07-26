#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "window.hpp"
#include "context.hpp"

class Swapchain
{
public:
	Swapchain (const Context& context, const Window& window);
	virtual ~Swapchain();

	void checkPresentModes();

	void checkImageCounts();

	void checkCurrentExtent();

	void checkFormatsAvailable();

	void createSwapchain();

	std::vector<vk::ImageView> imageViews;

	void createRenderPass();

	void createFramebuffers();

	std::vector<vk::Framebuffer> framebuffers;
	vk::Extent2D swapchainExtent;

	vk::RenderPass renderPass;

	uint32_t acquireNextImage(const vk::Semaphore&);

	void checkSurfaceCapabilities();

	const vk::Fence& getSubmitFence();

	void initializeImageFences();

	void prepareForRender();

	uint32_t currentIndex{0};

	vk::PresentInfoKHR presentInfo;

	const Window& window;
	 
	const Context& context;

	std::vector<vk::Image> images;

	vk::SwapchainKHR swapchain;

private:
	vk::SurfaceKHR surface;
	uint32_t queueFamilyIndex;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;
	vk::Format colorFormat;
	vk::ColorSpaceKHR colorSpace;
	vk::PresentModeKHR presentMode = vk::PresentModeKHR::eImmediate;
	vk::SurfaceCapabilitiesKHR surfCaps;
	bool swapchainCreated = false;
	std::vector<vk::Fence> imageFences;
	
	void createSurface();

	void getSurfaceCapabilities();
	
	void chooseSwapExtent();

	void chooseFormat();

	void grabImages();

	void createImageViews();

	void destroyImageViews();

	void destroyFramebuffers();

};

#endif /* ifndef SWAPCHAIN_H */
