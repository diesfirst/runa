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
	vk::Format colorFormat;
	vk::ColorSpaceKHR colorSpace;
	
	void createSurface();

	void setColorFormat();
	
	void getSurfaceCapabilities();

};
