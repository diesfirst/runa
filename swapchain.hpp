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
	
	void createSurface();
	
	void setGraphicsQueue(vk::Device device);

	void setPresentQueue(vk::Device device);
};
