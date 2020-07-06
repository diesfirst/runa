#include <render/renderframe.hpp>
#include <render/context.hpp>
#include <render/pipeline.hpp>
#include <render/attachment.hpp>

namespace sword
{

namespace render
{

RenderFrame::RenderFrame(
		const Context& context, 
		std::unique_ptr<Attachment>&& renderTarget,
		uint32_t width, uint32_t height) :
	swapchainAttachment{std::move(renderTarget)},
	commandPool{CommandPool(
            context.getDevice(), 
            context.getGraphicQueue(0), 
            context.getGraphicsQueueFamilyIndex(),
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer)},
	width{width},
	height{height}
{
    const auto& device = context.getDevice();
	vk::SemaphoreCreateInfo semaInfo;
	vk::FenceCreateInfo fenceInfo;
	semaphore = device.createSemaphoreUnique(semaInfo);
	fence = device.createFenceUnique(fenceInfo);

	vk::DescriptorPoolSize uboDynamicSize;
	vk::DescriptorPoolSize imageSamplerSize;
	vk::DescriptorPoolSize uboSize;
	uboDynamicSize.setType(vk::DescriptorType::eUniformBufferDynamic);
	uboDynamicSize.setDescriptorCount(1);
	imageSamplerSize.setType(vk::DescriptorType::eCombinedImageSampler);
	imageSamplerSize.setDescriptorCount(20); //arbitrary
	uboSize.setType(vk::DescriptorType::eUniformBuffer);
	uboSize.setDescriptorCount(1);

	std::array<vk::DescriptorPoolSize, 3> sizes{
		uboSize, imageSamplerSize, uboDynamicSize};

	vk::DescriptorPoolCreateInfo ci;
	ci.setMaxSets(1); //currently allow just one descriptor set
	ci.setPPoolSizes(sizes.data());
	ci.setPoolSizeCount(sizes.size());

	descriptorPool = device.createDescriptorPoolUnique(ci);
}

void RenderFrame::addRenderLayer(RenderLayer&& layer)
{
    renderLayers.emplace_back(std::move(layer));
}

void RenderFrame::addRenderLayer(
        const RenderPass& pass, 
        const GraphicsPipeline& pipe,
        const vk::Device& device,
        const DrawParms drawParms)
{
    assert(pipe.isCreated() && "Pipeline not created!");
    renderLayers.emplace_back(
                device,
                *swapchainAttachment,
                pass,
                pipe, 
                drawParms);
}

void RenderFrame::clearRenderPassInstances()
{
    renderLayers.clear();
}

CommandBuffer& RenderFrame::requestRenderBuffer(uint32_t bufferId)
{
    return commandPool.requestCommandBuffer(bufferId);
}

CommandBuffer& RenderFrame::getRenderBuffer(uint32_t bufferId)
{
    return commandPool.requestCommandBuffer(bufferId);
}

vk::Semaphore RenderFrame::requestSemaphore()
{
	return *semaphore;
}

Attachment& RenderFrame::getSwapAttachment()
{
    return *swapchainAttachment;
}

void RenderFrame::createDescriptorSets(
        const std::vector<vk::DescriptorSetLayout>& layouts)
{
	vk::DescriptorSetAllocateInfo ai;
	ai.setPSetLayouts(layouts.data());
	ai.setDescriptorSetCount(layouts.size());
	ai.setDescriptorPool(*descriptorPool);
	descriptorSets = descriptorPool.getOwner().allocateDescriptorSetsUnique(ai);
}

const std::vector<vk::UniqueDescriptorSet>& RenderFrame::getDescriptorSets() const
{
	return descriptorSets;
}

void RenderFrame::addUniformBufferBlock(BufferBlock* block)
{
    uniformBufferBlocks.push_back(block);
}

BufferBlock* RenderFrame::getUniformBufferBlock(int index) 
{
    assert(index < uniformBufferBlocks.size());
    return uniformBufferBlocks[index];
}

}; // namespace render

}; // namespace sword
