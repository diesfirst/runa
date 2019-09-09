#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "swapchain.hpp"
#include "window.hpp"

class Viewport
{
public:
	Viewport(const int width, const int height, Context& context);
	~Viewport();

	const uint32_t getWidth() const;
	const uint32_t getHeight() const;
	const vk::Extent2D getExtent() const;
	std::vector<vk::Image>& getSwapImages() const;

private:
	XWindow window;
	Swapchain swapchain;
	uint32_t width, height;
	vk::Extent2D extent;

};

#endif /* VIEWPORT_H */
