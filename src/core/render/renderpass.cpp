#include <render/renderpass.hpp>

namespace sword
{

namespace render
{

RenderPass::RenderPass(const vk::Device& device, const std::string name) :
	device{device},
	name{name}
{
}

//RenderPass::RenderPass(RenderPass&& other) :
//	device{other.device},
//	name{other.name},
//	handle{other.handle},
//	attachments{std::move(other.attachments)},
//	colorAttachments{std::move(other.colorAttachments)},
//	references{std::move(other.references)},
//	subpasses{std::move(other.subpasses)},
//	subpassDependencies{std::move(other.subpassDependencies)}
//{
//	other.handle = nullptr;
//	other.attachments.clear();
//	other.colorAttachments.clear();
//	other.references.clear();
//	other.subpasses.clear();
//	other.subpassDependencies.clear();
//}
//
//bool RenderPass::operator<(const RenderPass& rhs)
//{
//	return handle == rhs.handle;
//}
//
void RenderPass::createColorAttachment(
		const vk::Format format,
		const vk::ImageLayout initialLayout,
		const vk::ImageLayout finalLayout,
        vk::ClearColorValue clearColor,
        vk::AttachmentLoadOp loadOp)
{
	vk::AttachmentDescription attachment;
	attachment.setFormat(format);
	attachment.setSamples(vk::SampleCountFlagBits::e1);
	//Sets what to do with data in the attachment
	//before rendering
    attachment.setLoadOp(loadOp);
	attachment.setInitialLayout(initialLayout);
	//Sets what we do with the data after rendering
	//We want to show it so we will store it
	attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
	attachment.setFinalLayout(finalLayout);

	colorAttachments.push_back(attachment);
	attachments.push_back(attachment);

	vk::AttachmentReference colorRef;
	colorRef.setAttachment(0); //our first attachment is color
	colorRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	references.push_back(colorRef);
    clearValue.setColor(clearColor);
}

void RenderPass::createBasicSubpassDependency()
{
	vk::SubpassDependency d0;
	d0.setSrcSubpass(VK_SUBPASS_EXTERNAL);
	d0.setDstSubpass(0);
	d0.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
	d0.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
	d0.setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
	d0.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	d0.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
	subpassDependencies.push_back(d0);

	vk::SubpassDependency d1;
	d1.setSrcSubpass(0);
	d1.setDstSubpass(VK_SUBPASS_EXTERNAL);
	d1.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
	d1.setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
	d1.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	d1.setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
	d1.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
	subpassDependencies.push_back(d1);
}

void RenderPass::addSubpassDependency(vk::SubpassDependency d)
{
	subpassDependencies.push_back(d);
}

void RenderPass::createSubpass()
{
	vk::SubpassDescription subpass;
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpass.setColorAttachmentCount(colorAttachments.size());
	subpass.setPColorAttachments(references.data());
	subpass.setPResolveAttachments(nullptr);
	subpass.setInputAttachmentCount(0);
	subpass.setPInputAttachments(nullptr);
	subpass.setPreserveAttachmentCount(0);
	subpass.setPPreserveAttachments(nullptr);
	subpass.setPDepthStencilAttachment(nullptr);
	subpasses.push_back(subpass);
}

void RenderPass::create()
{
	//if any of the sizes are 0, then the corresponding data
	//is undefined. but since the counts are 0, this should
	//be ok
	vk::RenderPassCreateInfo createInfo;
	createInfo.setAttachmentCount(attachments.size());
	createInfo.setPAttachments(attachments.data());
	assert(subpasses.size() != 0 && "Must be at least one subpass");
	createInfo.setSubpassCount(subpasses.size());
	createInfo.setPSubpasses(subpasses.data());
	createInfo.setDependencyCount(subpassDependencies.size());
	createInfo.setPDependencies(subpassDependencies.data());
  	handle = device.createRenderPassUnique(createInfo);
	created = true;
}

bool RenderPass::isCreated() const
{
	return created;
}

const vk::ClearValue* RenderPass::getClearValue() const
{
    return &clearValue;
}

const vk::RenderPass& RenderPass::getHandle() const
{
	return *handle;
}


}; // namespace render

}; // namespace sword
