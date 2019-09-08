#ifndef RENDERER_H
#define RENDERER_H

#include "context.hpp"
#include "pipe.hpp"

class Swapchain;

class Renderer
{
public:
	Renderer(const Context&, Pipe&, const uint32_t width, const uint32_t height);
	virtual ~Renderer();
	void createRenderPass(vk::Format);
  void createFramebuffers(const Swapchain&);

	vk::RenderPass renderPass;
	std::vector<vk::Framebuffer> framebuffers;

private:
	const Context& context;
	Pipe& pipe;
	uint32_t width;
	uint32_t height;

	void destroyFramebuffers();
};

#endif /* RENDERER_H */
