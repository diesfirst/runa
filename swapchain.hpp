#include "context.hpp"
#include "window.hpp"

class Swapchain
{
public:
	Swapchain (const Context& context);
	virtual ~Swapchain();

	void checkPresentModes();

	void checkImageCounts();

	void checkCurrentExtent();

	void createSwapchain();

private:
	const Context& context;
	vk::SurfaceKHR surface;
	Window window;
	uint32_t queueFamilyIndex;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;
	vk::Format colorFormat = vk::Format::eB8G8R8A8Snorm;
	vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
	vk::Extent2D swapchainExtent;
	vk::SurfaceCapabilitiesKHR surfCaps;
	vk::SwapchainKHR swapchain;
	
	void createSurface();

	void setColorFormat();
	
	void getSurfaceCapabilities();
	
	void chooseSwapExtent();

	void chooseFormat();
};
