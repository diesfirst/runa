#include "context.hpp"
#include "window.hpp"

class Swapchain
{
public:
	Swapchain (const Context& context);
	virtual ~Swapchain();

private:
	const Context& context;
	vk::SurfaceKHR surface;
	Window window;
	uint32_t queueFamilyIndex;
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;
	vk::Format colorFormat = vk::Format::eB8G8R8A8Snorm;
	vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	
	void createSurface();

	void setColorFormat();
	
	void getSurfaceCapabilities();

};
