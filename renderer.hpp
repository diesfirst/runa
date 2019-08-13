#ifndef RENDERER_H
#define RENDERER_H

#include "context.hpp"

class Swapchain;

class Renderer
{
public:
	Renderer(const Context&);
	virtual ~Renderer();
	void createRenderPass(vk::Format);
  void createFramebuffers(const Swapchain&);

	vk::RenderPass renderPass;
	std::vector<vk::Framebuffer> framebuffers;

private:
	const Context& context;

	void destroyFramebuffers();
};

#endif /* RENDERER_H */
