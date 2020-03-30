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
	context(context),
	device(context.device),
	swapchainAttachment{std::move(renderTarget)},
	commandPool{CommandPool(
            device, 
            context.queue, 
            context.getGraphicsQueueFamilyIndex(),
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer)},
	width{width},
	height{height}
{
	vk::SemaphoreCreateInfo semaInfo;
	vk::FenceCreateInfo fenceInfo;
	semaphore = device.createSemaphore(semaInfo);
	fence = device.createFence(fenceInfo);
	createDescriptorPool();
}

RenderFrame::~RenderFrame()
{
	if (semaphore)
		device.destroySemaphore(semaphore);
	if (fence)	
		device.destroyFence(fence);
	if (descriptorPool)
		device.destroyDescriptorPool(descriptorPool);
}

RenderFrame::RenderFrame(RenderFrame&& other) :
	context(other.context),
	device(other.device),
	commandPool(std::move(other.commandPool)),
	fence(other.fence),
	semaphore(other.semaphore),
	descriptorPool(other.descriptorPool),
	descriptorSets{std::move(other.descriptorSets)},
	width{other.width},
	height{other.height},
	commandBuffers{std::move(other.commandBuffers)},
	swapchainAttachment{std::move(other.swapchainAttachment)},
    renderLayers{std::move(other.renderLayers)},
    bufferBlock{other.bufferBlock}
{
	other.semaphore = nullptr;
	other.fence = nullptr;
	other.descriptorPool = nullptr;
    other.renderLayers.clear();
}

void RenderFrame::addRenderPassInstance(
        Attachment& target, 
        const RenderPass& pass, 
        const GraphicsPipeline& pipe)
{
    assert(pipe.isCreated() && "Pipeline not created!");
    renderLayers.emplace_back(
                device,
                target,
                pass,
                pipe);
}

void RenderFrame::addRenderPassInstance(
        const RenderPass& pass, 
        const GraphicsPipeline& pipe)
{
    assert(pipe.isCreated() && "Pipeline not created!");
    renderLayers.emplace_back(
                device,
                *swapchainAttachment,
                pass,
                pipe);
}

void RenderFrame::clearRenderPassInstances()
{
    renderLayers.clear();
}

CommandBuffer& RenderFrame::requestRenderBuffer(uint32_t bufferId)
{
    if (bufferId < commandBuffers.size())
    {
        auto& buffer = *commandBuffers.at(bufferId);
        buffer.reset();
        return buffer;
    }
    else
    {
        auto cmdPtr = &commandPool.requestCommandBuffer();
        commandBuffers.push_back(cmdPtr);
        return *commandBuffers.back();
    }
}

CommandBuffer& RenderFrame::getRenderBuffer(uint32_t bufferId)
{
    return *commandBuffers.at(bufferId);
}

vk::Semaphore RenderFrame::requestSemaphore()
{
	return semaphore;
}

void RenderFrame::createDescriptorPool()
{
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

	descriptorPool = device.createDescriptorPool(ci);
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
	ai.setDescriptorPool(descriptorPool);
	descriptorSets = device.allocateDescriptorSets(ai);
}

const std::vector<vk::DescriptorSet>& RenderFrame::getDescriptorSets() const
{
	return descriptorSets;
}

}; // namespace render

}; // namespace sword
