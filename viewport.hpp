// Holds a swapchain and a window. 
#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "swapchain.hpp"
#include "window.hpp"

constexpr int WIDTH = 800;
constexpr int HEIGHT = 800;

class Viewport
{
public:
	Viewport(Context& context, const int width = WIDTH, const int height = HEIGHT);
	~Viewport();

	uint32_t getWidth() const;
	uint32_t getHeight() const;
	uint8_t getSwapImageCount() const;
	vk::Extent2D getExtent() const;
	const std::vector<vk::ImageView>& getSwapImageViews() const;
	vk::ImageView getSwapImageView(uint32_t i) const;

private:
	XWindow window;
	Swapchain swapchain;
	uint32_t width, height;
	vk::Extent2D extent;
	vk::Viewport viewport;

};

#endif /* VIEWPORT_H */
