//storage space for now, but may be a class for 3D to 2D conversions
#include <vector>
#include "renderer.hpp"
#include "swapchain.hpp"

Renderer::Renderer(
		const Context& context, 
		Pipe& pipe, 
		const uint32_t width,
		const uint32_t height) :
	context(context),
	pipe(pipe),
	width(width),
	height(height)
{
	createRenderPass(vk::Format::eB8G8R8A8Unorm); //by inspection
	pipe.createGraphicsPipeline(renderPass, width, height);
}

Renderer::~Renderer()
{
	context.device.destroyRenderPass(renderPass);
	for (auto framebuffer : framebuffers) 
	{
		context.device.destroyFramebuffer(framebuffer);
	}
}

void Renderer::createRenderPass(vk::Format colorFormat)
{
	std::vector<vk::AttachmentDescription> attachments;
	std::vector<vk::AttachmentReference> references;
	std::vector<vk::SubpassDescription> subpasses;
	std::vector<vk::SubpassDependency> subpassDependencies;

	vk::AttachmentDescription attachment;
	attachment.setFormat(colorFormat);
	attachment.setSamples(vk::SampleCountFlagBits::e1);
	//Sets what to do with data in the attachment
	//before rendering
	attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
	//Sets what we do with the data after rendering
	//We want to show it so we will store it
	attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
	attachment.setInitialLayout(vk::ImageLayout::eUndefined);
	attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	attachments.push_back(attachment);

	vk::AttachmentReference colorRef;
	colorRef.setAttachment(0); //our first attachment is color
	colorRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	references.push_back(colorRef);

	vk::SubpassDescription subpass;
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpass.setColorAttachmentCount(1);
	subpass.setPColorAttachments(references.data());
	subpasses.push_back(subpass);

	vk::SubpassDependency dependency;
	dependency.setSrcSubpass(
			VK_SUBPASS_EXTERNAL);
	dependency.setSrcAccessMask(
			vk::AccessFlagBits::eColorAttachmentWrite);
	dependency.setSrcStageMask(
			vk::PipelineStageFlagBits::eColorAttachmentOutput);
	dependency.setDstSubpass(
			0);
	dependency.setDstAccessMask(
			vk::AccessFlagBits::eColorAttachmentRead);
	dependency.setDstStageMask(
			vk::PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependencies.push_back(dependency);

	vk::RenderPassCreateInfo createInfo;
	createInfo.setAttachmentCount(attachments.size());
	createInfo.setPAttachments(attachments.data());
	createInfo.setSubpassCount(subpasses.size());
	createInfo.setPSubpasses(subpasses.data());
	createInfo.setDependencyCount(subpassDependencies.size());
	createInfo.setPDependencies(subpassDependencies.data());
  renderPass = context.device.createRenderPass(createInfo);
}


void Renderer::createFramebuffers(const Swapchain& swapchain)
{
	vk::FramebufferCreateInfo createInfo;
	createInfo.setWidth(swapchain.extent.width);
	createInfo.setHeight(swapchain.extent.height);
	createInfo.setRenderPass(renderPass); 
	createInfo.setAttachmentCount(1);
	createInfo.setLayers(1);
	std::array<vk::ImageView, 1> imageViewsTemp;
	createInfo.setPAttachments(imageViewsTemp.data());
	for (const auto image : swapchain.imageViews) {
		imageViewsTemp[0] = image;
		framebuffers.push_back(
				context.device.createFramebuffer(createInfo));
	}
}

void Renderer::destroyFramebuffers()
{
	for (auto framebuffer : framebuffers) {
		context.device.destroyFramebuffer(framebuffer);
	}
}
