#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "context.hpp"

class XWindow; //forward declaration

class Swapchain
{
public:
	Swapchain (const Context& context, const XWindow& window, const uint8_t swapImageCount);

	friend class Viewport;

	virtual ~Swapchain();

	void checkPresentModes();

	void checkFormatsAvailable();

	void createSwapchain();

	void createFramebuffers();

	uint32_t acquireNextImage(const vk::Semaphore&);

	uint8_t getCurrentIndex() const;

	uint8_t getImageCount() const;

	void incrementIndex();



private:
	vk::ColorSpaceKHR colorSpace;
	vk::PresentModeKHR presentMode;
	const XWindow& window;
	const Context& context;
	std::vector<vk::Image> images;
	std::vector<vk::ImageView> imageViews;
	vk::SwapchainKHR swapchain;
	vk::SurfaceCapabilitiesKHR surfCaps;
	vk::SurfaceKHR surface;
	std::vector<vk::Framebuffer> framebuffers;
	vk::Extent2D extent;
	uint32_t currentIndex{0};
	vk::Format colorFormat;
	uint8_t imageCount;

	bool swapchainCreated = false;

	void setImageCount(const uint32_t count = 3);

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
