#include "description.hpp"
#include <cstring>
#include <bitset>
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

Description::Description(Context& context) :
	context(context)
{
	//arbitrary size for now...
	//could be more performant to wrap the index and vertex buffer
	//into a single uber buffer
	vertexBlock = context.getVertexBlock(10000);
	indexBlock = context.getIndexBlock(10000);
	vertices = static_cast<Point*>(vertexBlock->pHostMemory);
	indices = static_cast<uint32_t*>(indexBlock->pHostMemory);
	initDescriptorSetLayout();
	//one descriptor set for world transform
}

Description::~Description()
{
	context.device.destroyDescriptorPool(descriptorPool);
	context.device.destroyDescriptorSetLayout(descriptorSetLayout);
}

void Description::createCamera(const uint16_t width, const uint16_t height)
{
	cameras.push_back(std::make_shared<Camera>(width, height));
	occupants.push_back(cameras.back());
	if (cameras.size() == 1) //no previous cameras existed
	{
		curCamera = cameras[0];
	}
}

void Description::loadImage(std::string path)
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	assert (pixels);
	texture1 = std::make_unique<Image>(&context, width, height, 4);
	texture1->loadArray(pixels, width * height * 4);
}

vk::Buffer& Description::getVkVertexBuffer()
{
	return vertexBlock->buffer;
}

vk::Buffer& Description::getVkIndexBuffer()
{
	return indexBlock->buffer;
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

uint32_t Description::getMeshCount()
{
	return meshes.size();
}

uint32_t Description::getOccupantCount()
{
	return occupants.size();
}

uint32_t Description::getDynamicAlignment()
{
	assert (dynamicAlignment != 0);
	return dynamicAlignment;
}

std::vector<DrawableInfo>& Description::getDrawInfos()
{
	return drawInfos;
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

void Description::updateVertexBuffer(const PointBased& newPointBased)
{
	uint32_t nPoints =  newPointBased.points.size();
	assert(vertices != nullptr);
	std::memcpy(
			vertices + curVertOffset,
			newPointBased.points.data(),
			sizeof(Point) * nPoints
			);
	curVertOffset += nPoints;
}

void Description::updateIndexBuffer(const Mesh& newMesh)
{
	uint32_t nIndices = newMesh.indices.size();
	std::memcpy(
			indices + curIndexOffset,
			newMesh.indices.data(),
			sizeof(uint32_t) * nIndices);
	drawInfos.push_back({curIndexOffset, nIndices, curVertOffset});
	curIndexOffset += nIndices;
}

void Description::initDescriptorSetLayout()
{
	std::array<vk::DescriptorSetLayoutBinding, 2> layoutBindings;

	layoutBindings[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
	layoutBindings[0].setBinding(0);
	layoutBindings[0].setDescriptorCount(1);
	layoutBindings[0].setStageFlags(vk::ShaderStageFlagBits::eVertex);
	layoutBindings[0].setPImmutableSamplers(nullptr);

	layoutBindings[1].setDescriptorType(
			vk::DescriptorType::eUniformBufferDynamic);
	layoutBindings[1].setBinding(1);
	layoutBindings[1].setDescriptorCount(1);
	layoutBindings[1].setStageFlags(vk::ShaderStageFlagBits::eVertex);
	layoutBindings[1].setPImmutableSamplers(nullptr);

	vk::DescriptorSetLayoutCreateInfo info;
	info.setPBindings(layoutBindings.data());
	info.setBindingCount(layoutBindings.size());
	descriptorSetLayout = context.device.createDescriptorSetLayout(info);
}

void Description::createDescriptorPool(uint32_t swapImageCount)
{
	std::array<vk::DescriptorPoolSize, 2> poolSizes;

	poolSizes[0].setType(vk::DescriptorType::eUniformBuffer);
	poolSizes[0].setDescriptorCount(swapImageCount); 
	//specifies total descriptors allocated across all sets
	//we have 1 descriptor for each set, but swapImageCount sets
	//so we set this to swapImageCount

	poolSizes[1].setType(vk::DescriptorType::eUniformBufferDynamic);
	poolSizes[1].setDescriptorCount(swapImageCount);

	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.setPoolSizeCount(poolSizes.size());
	poolInfo.setPPoolSizes(poolSizes.data());
	poolInfo.setMaxSets(swapImageCount);

	descriptorSetCount = swapImageCount;
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

	for (uint32_t i = 0; i < count; ++i) 
	{
		std::array<vk::WriteDescriptorSet, 2> writes;

		vk::DescriptorBufferInfo uboBufferInfo;
		uboBufferInfo.setBuffer(uboBlocks[i]->buffer);
		uboBufferInfo.setOffset(0);
		uboBufferInfo.setRange(VK_WHOLE_SIZE);
		
		writes[0].setDstSet(descriptorSets[i]);
		writes[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
		writes[0].setDstBinding(0);
		writes[0].setDstArrayElement(0);
		writes[0].setDescriptorCount(1);
		writes[0].setPBufferInfo(&uboBufferInfo);

		vk::DescriptorBufferInfo uboDynBufferInfo;
		uboDynBufferInfo.setBuffer(uboDynamicBlocks[i]->buffer);
		uboDynBufferInfo.setOffset(0);
		uboDynBufferInfo.setRange(dynamicAlignment);
		//set to size of the ubo dynamic data per model

		writes[1].setDstSet(descriptorSets[i]);
		writes[1].setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
		writes[1].setDstBinding(1);
		writes[1].setDstArrayElement(0);
		writes[1].setDescriptorCount(1);
		writes[1].setPBufferInfo(&uboDynBufferInfo);

		context.device.updateDescriptorSets(writes, nullptr);
	}
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
	createDescriptorPool(count);
	createDescriptorSets(count);
	descriptorsPrepared = true;
}

void Description::prepareUniformBuffers(const uint32_t count)
{
	size_t minUboAlignment = 
		context.physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
	assert (minUboAlignment > 0);
	size_t dA = sizeof(glm::mat4);
	dA = (dA + minUboAlignment - 1) & ~(minUboAlignment - 1);
	size_t bufferSize = MAX_OCCUPANTS * dA;
	dynamicAlignment = dA;
	uboDynamicData.model = (glm::mat4*)aligned_alloc(minUboAlignment, bufferSize);

	uboBlocks = 
		context.pMemory->createUBOBlocks(count, sizeof(UboVS));
	uboDynamicBlocks = 
		context.pMemory->createDynamicUBOBlocks(count, bufferSize);

	updateUniformBuffers(); //initial swapIndex assumed 0
	updateDynamicUniformBuffers();//initial swapIndex assumed 0
}

void Description::updateAllCurrentUbos()
{
	updateUniformBuffers();
	updateDynamicUniformBuffers();
}

void Description::update(PointBased* pb)
{
	uint32_t vertOffset = pb->getVertOffset();
	uint32_t range = pb->points.size();
	std::memcpy(
			vertices + vertOffset,
			pb->points.data(),
			sizeof(Point) * range);
}

void Description::updateUniformBuffers()
{
	assert (curCamera.get() != nullptr);
	uboView.projection = curCamera->projection;
	uboView.view = curCamera->view;
	memcpy(uboBlocks[curSwapIndex]->pHostMemory, &uboView, sizeof(UboVS));
}

void Description::updateDynamicUniformBuffers()
{
	uint32_t nMeshes = getMeshCount();
	char* pModelMemory = (char*)uboDynamicData.model;
	for (size_t i = 0; i < nMeshes; ++i) 
	{
		glm::mat4* mat = (glm::mat4*)(pModelMemory + i * dynamicAlignment);
		*mat = meshes[i]->getTransform();
	}
	memcpy(uboDynamicBlocks[curSwapIndex]->pHostMemory, uboDynamicData.model,
		nMeshes * dynamicAlignment);	
}

void Description::setCurrentSwapIndex(uint8_t curIndex)
{
	curSwapIndex = curIndex;
}
