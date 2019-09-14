#include "description.hpp"

Description::Description(Context& context) :
	context(context)
{
	prepareDescriptorSets(1); //one descriptor set for world transform
}

Description::~Description()
{
}

void Description::initDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding binding;
	binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
	binding.setBinding(0);
	//possible to use an array of uniform buffer objects
	//could be used for skeletal animation
	binding.setDescriptorCount(1);
	binding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
	binding.setPImmutableSamplers(nullptr);

	vk::DescriptorSetLayoutCreateInfo info;
	info.setPBindings(&binding);
	info.setBindingCount(1);
	descriptorSetLayout = context.device.createDescriptorSetLayout(info);
}

void Description::createDescriptorPool(uint32_t descriptorCount)
{
	vk::DescriptorPoolSize poolSize;
	poolSize.setType(vk::DescriptorType::eUniformBuffer);
	poolSize.setDescriptorCount(descriptorCount);

	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.setPoolSizeCount(1);
	poolInfo.setPPoolSizes(&poolSize);
	poolInfo.setMaxSets(descriptorCount);

	descriptorCount = descriptorCount;
	descriptorPool = context.device.createDescriptorPool(poolInfo);
}

void Description::createDescriptorSets(uint32_t count)
{
	std::vector<vk::DescriptorSetLayout> layouts(count, descriptorSetLayout);

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.setDescriptorPool(descriptorPool);
	allocInfo.setDescriptorSetCount(count);
	allocInfo.setPSetLayouts(layouts.data());

	descriptorSets.resize(count);
	descriptorSets = context.device.allocateDescriptorSets(allocInfo);
}

void Description::updateDescriptorSets(
			uint32_t count, 
			std::vector<bufferBlock>* uboBlocks,
			size_t uboSize)
{
	assert(descriptorsPrepared);

	vk::DescriptorBufferInfo bufferInfo;
	bufferInfo.setOffset(0);
	bufferInfo.setRange(uboSize);

	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
	descriptorWrite.setDescriptorCount(1);
	descriptorWrite.setDstBinding(0);
	descriptorWrite.setDstArrayElement(0);
	descriptorWrite.setPBufferInfo(&bufferInfo);
	descriptorWrite.setPImageInfo(nullptr);
	descriptorWrite.setPTexelBufferView(nullptr);

	for (int i = 0; i < count; ++i) 
	{
		bufferInfo.setBuffer((*uboBlocks)[i].buffer);
		descriptorWrite.setDstSet(descriptorSets[i]);
		context.device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
	}
}

void Description::prepareDescriptorSets(uint32_t count)
{
	initDescriptorSetLayout();
	createDescriptorPool(count);
	createDescriptorSets(count);
	descriptorsPrepared = true;
}

