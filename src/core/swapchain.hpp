#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "context.hpp"
#include <tuple>

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

	std::vector<vk::Image>& getImages();

	vk::Extent2D getExtent2D();

	vk::Extent3D getExtent3D();

	vk::Format getFormat();

	vk::ImageUsageFlags getUsageFlags();

	std::tuple<uint32_t, vk::Semaphore*> acquireNextImageNoFence();

	uint32_t acquireNextImage(vk::Semaphore, vk::Fence);

	uint8_t getCurrentIndex() const;

	uint8_t getImageCount() const;

	const vk::SwapchainKHR& getHandle() const;

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
	vk::Extent2D extent;
	uint32_t currentIndex{0};
	vk::Format colorFormat;
	vk::ImageUsageFlags usageFlags;
	uint8_t imageCount;

	bool swapchainCreated = false;

	void initSyncObjects();

	void setImageCount(const uint32_t count = 3);

	void setQueueFamilyIndex();
	
	void createSurface();

	void setSurfaceCapabilities();
	
	void setSwapExtent();

	void setFormat();

	void setImages();

	void setPresentMode();

	void createImageViews();

	void destroyImageViews();

	void destroySyncObjects();


};

#endif /* ifndef SWAPCHAIN_H */
