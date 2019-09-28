#include "description.hpp"
#include <cstring>

Description::Description(Context& context) :
	context(context)
{
	vertexBlock = context.getVertexBlock(6000);
	vertices = static_cast<Point*>(vertexBlock->pHostMemory);
	//arbitrary size for now...
	prepareDescriptorSets(3); 
	//one descriptor set for world transform
}

Description::~Description()
{
	context.device.destroyDescriptorPool(descriptorPool);
	context.device.destroyDescriptorSetLayout(descriptorSetLayout);
}

void Description::createTriangle()
{
	geometry.push_back(std::make_shared<Triangle>());
	pointBasedOccupants.push_back(geometry.back());
	occupants.push_back(geometry.back());
	updateVertexBuffer();
}

void Description::createTriangle(Point p0, Point p1, Point p2)
{
	geometry.push_back(std::make_shared<Triangle>(p0, p1, p2));
	pointBasedOccupants.push_back(geometry.back());
	occupants.push_back(geometry.back());
	updateVertexBuffer();
}

vk::Buffer& Description::getVkVertexBuffer()
{
	return vertexBlock->buffer;
}

uint32_t Description::getVertexCount()
{
	uint32_t nPoints = 0;
	for (auto pointGeo : pointBasedOccupants) 
	{
		nPoints += pointGeo->points.size();
	}
	return nPoints;
}

vk::DescriptorSetLayout* Description::getPDescriptorSetLayout()
{
	return &descriptorSetLayout;
}

vk::VertexInputBindingDescription Description::getBindingDescription()
{
	vk::VertexInputBindingDescription description;
	description.setBinding(0);
	description.setStride(sizeof(Point));
	description.setInputRate(vk::VertexInputRate::eVertex);
	return description;
}
	
std::array<vk::VertexInputAttributeDescription, 2> 
	Description::getAttributeDescriptions()
{
	std::array<vk::VertexInputAttributeDescription, 2> descriptions;
	descriptions[0].binding = 0;
	descriptions[0].location = 0;
	descriptions[0].format = vk::Format::eR32G32B32Sfloat;
	descriptions[0].offset = offsetof(Point, pos); //where is this macro from?
	descriptions[1].binding = 0;
	descriptions[1].location = 1;
	descriptions[1].format = vk::Format::eR32G32B32Sfloat;
	descriptions[1].offset = offsetof(Point, color); //where is this macro from?
	return descriptions;
}

void Description::updateCommandBuffer()
{
}

void Description::updateVertexBuffer()
{
	uint32_t offset = 0;
	for (auto pItem : pointBasedOccupants) 
	{
		std::memcpy(
				vertices + offset, 
				pItem->points.data(), 
				sizeof(Point) * pItem->points.size());
		offset += pItem->points.size();
	}
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
			std::vector<BufferBlock>* uboBlocks,
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

	for (size_t i = 0; i < count; ++i) 
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

