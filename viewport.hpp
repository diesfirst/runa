#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "swapchain.hpp"
#include "window.hpp"

class Viewport
{
public:
	Viewport(const int width, const int height, Context context);
	~Viewport();

private:
	XWindow window;
	Swapchain swapchain;
	uint32_t width, height;

};

#endif /* VIEWPORT_H */
