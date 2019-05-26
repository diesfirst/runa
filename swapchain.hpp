#include "context.hpp"

class Swapchain
{
public:
	Swapchain (const Context& context);
	virtual ~Swapchain ();

private:
	const Context& context;
	vk::SurfaceKHR surface;
	
	void createSurface();
};
