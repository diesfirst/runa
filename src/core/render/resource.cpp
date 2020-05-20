#include <cstring>
#include <iostream>
#include <render/resource.hpp>

namespace sword
{

namespace render
{

Buffer::Buffer(
        const vk::Device& device,
        const vk::PhysicalDeviceProperties& props,
		const vk::PhysicalDeviceMemoryProperties& memProps,
		uint32_t size, 
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags typeFlags) :
	device{device},
    devProps{props},
    memProps{memProps},
	size{size}
{
	std::cout << "Creating buffer!" << std::endl;
	std::cout << "MM Size: " << size << std::endl;

	vk::BufferCreateInfo bufferInfo;
	bufferInfo.setSize(size);
	bufferInfo.setUsage(usage);
	bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

	handle = device.createBuffer(bufferInfo);

	auto memReqs = device.getBufferMemoryRequirements(handle);
//	to do: fix this, its not working
//	uint32_t memoryTypeIndex = findMemoryType(
//			memReqs.memoryTypeBits, typeFlags); 
//	std::cout << "Memory Type index: " << memoryTypeIndex << std::endl;
//
	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setMemoryTypeIndex(9); //always host visible for now
	allocInfo.setAllocationSize(memReqs.size);
//	allocInfo.setMemoryTypeIndex(memoryTypeIndex);
	std::cout << "Mem reqs size:" << memReqs.size << std::endl;
	std::cout << "Mem reqs alignment:" << memReqs.alignment << std::endl;

	memory = device.allocateMemory(allocInfo);

	device.bindBufferMemory(handle, memory, 0);
}


Buffer::~Buffer()
{
	device.unmapMemory(memory);
	device.destroyBuffer(handle);
	device.freeMemory(memory);
}

vk::Buffer& Buffer::getHandle()
{
	return handle;
}

BufferBlock* Buffer::requestBlock(uint32_t blockSize)
{
    uint32_t allocSize{0};
    uint32_t minAlignment = devProps.limits.minUniformBufferOffsetAlignment;
    std::cout << "min alignment " << minAlignment << std::endl;
    if (blockSize % minAlignment == 0) //is aligned
        allocSize = blockSize;
    else 
    {
        uint32_t nAlignmentsThatContainSize = (blockSize / minAlignment) + 1; 
        allocSize = nAlignmentsThatContainSize * minAlignment;
    }
    assert(allocSize < size && "No room for new block");
    auto block = std::make_unique<BufferBlock>();
    block->offset = curBlockOffset;
    block->size = blockSize;
    block->allocSize = allocSize;
    block->index = bufferBlocks.size();
    if (isMapped)
    {
        block->pHostMemory = static_cast<uint8_t*>(pHostMemory) + block->offset;
        block->isMapped = true;
        std::cout << "Block hostmem: " << block->pHostMemory << std::endl;
        std::cout << "===!+!+!+!+!+!+!+!+!+!+!+!++!+!+!+!+++===" << std::endl;
    }
    bufferBlocks.push_back(std::move(block));
    std::cout << " Block made" <<
       " curBlockOffset: " << curBlockOffset <<
       " blockSize: " << blockSize << std::endl;
    curBlockOffset += allocSize;
    return bufferBlocks.back().get();
}

void Buffer::popBackBlock()
{
    curBlockOffset -= bufferBlocks.back()->allocSize;
    bufferBlocks.pop_back();
    std::cout << "Block popped. " << bufferBlocks.size() << " remain." << std::endl;
}

void Buffer::map()
{
	assert(!isMapped);
	pHostMemory = device.mapMemory(memory, 0, VK_WHOLE_SIZE);
	assert(pHostMemory != (void*)VK_ERROR_MEMORY_MAP_FAILED);
    isMapped = true;
//
//    for (auto& block : bufferBlocks) 
//    {
//        block->pHostMemory = static_cast<uint8_t*>(pHostMemory) + block->offset;
//        block->isMapped = true;
//        std::cout << "Block hostmem: " << block->pHostMemory << std::endl;
//        std::cout << "===!+!+!+!+!+!+!+!+!+!+!+!++!+!+!+!+++===" << std::endl;
//    }
}

void Buffer::unmap()
{
	assert(isMapped);
	device.unmapMemory(memory);
    for (auto& block : bufferBlocks) 
    {
        block->pHostMemory = nullptr;
    }
	isMapped = false;
}

uint32_t Buffer::findMemoryType(
		uint32_t typeFilter, 
		vk::MemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) 
	{
		if (
			(typeFilter & (1 << i)) && 
			(memProps.memoryTypes[i].propertyFlags & properties) == properties
			) 
		{
		return i;
	    }
	}

	throw std::runtime_error("Failed to find suitable memory");
}

Image::Image(
		const vk::Device& device,
		const vk::Extent3D extent,
		const vk::Format format,
		const vk::ImageUsageFlags usageFlags,
		const vk::ImageLayout initialLayout) :
	device(device),
	extent{extent},
	format{format},
	usageFlags{usageFlags}
{
	vk::ImageCreateInfo createInfo;
	createInfo.setImageType(vk::ImageType::e2D);
	createInfo.setExtent(extent);
	createInfo.setMipLevels(1);
	createInfo.setArrayLayers(1);
	createInfo.setFormat(format);
	createInfo.setTiling(vk::ImageTiling::eOptimal);
	createInfo.setInitialLayout(initialLayout);
	createInfo.setUsage(usageFlags);
	createInfo.setSamples(vk::SampleCountFlagBits::e1);
	createInfo.setSharingMode(vk::SharingMode::eExclusive);
	handle = device.createImage(createInfo);

	auto imgMemReq = device.getImageMemoryRequirements(handle); 

	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.setAllocationSize(imgMemReq.size);
	memAllocInfo.setMemoryTypeIndex(7); //device local

	memory  = device.allocateMemory(memAllocInfo);

	device.bindImageMemory(handle, memory, 0);

	vk::ComponentMapping components;
	components.setA(vk::ComponentSwizzle::eIdentity);
	components.setB(vk::ComponentSwizzle::eIdentity);
	components.setG(vk::ComponentSwizzle::eIdentity);
	components.setR(vk::ComponentSwizzle::eIdentity);

	vk::ImageSubresourceRange subResRange;
	subResRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subResRange.setLayerCount(1);
	subResRange.setLevelCount(1);
	subResRange.setBaseMipLevel(0);
	subResRange.setBaseArrayLayer(0);
	
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.setImage(this->handle);
	viewInfo.setFormat(format);
	viewInfo.setViewType(vk::ImageViewType::e2D);
	viewInfo.setComponents(components);
	viewInfo.setSubresourceRange(subResRange);
	view = device.createImageView(viewInfo);

	vk::SamplerCreateInfo samplerInfo;
	//might want to try nearest at some point
	samplerInfo.setMagFilter(vk::Filter::eLinear);
	samplerInfo.setMinFilter(vk::Filter::eLinear);
	samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
	samplerInfo.setAddressModeU(vk::SamplerAddressMode::eClampToBorder);
	samplerInfo.setAddressModeV(vk::SamplerAddressMode::eClampToBorder);
	samplerInfo.setAddressModeW(vk::SamplerAddressMode::eClampToBorder);
	samplerInfo.setMipLodBias(0.0);
	samplerInfo.setMaxAnisotropy(1.0);
	samplerInfo.setMinLod(0.0);
	samplerInfo.setMaxLod(1.0);
	samplerInfo.setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
	sampler = device.createSampler(samplerInfo);
}

Image::Image(	
		const vk::Device& device, 
		const vk::Image handle, 
		const vk::Extent3D extent, 
		vk::Format format, 
		vk::ImageUsageFlags usageFlags) :
	device(device),
	handle{handle},
	extent(extent),
	format(format),
	usageFlags(usageFlags)
{
	selfManaged = false;
	vk::ComponentMapping components;
	components.setA(vk::ComponentSwizzle::eIdentity);
	components.setB(vk::ComponentSwizzle::eIdentity);
	components.setG(vk::ComponentSwizzle::eIdentity);
	components.setR(vk::ComponentSwizzle::eIdentity);

	vk::ImageSubresourceRange subResRange;
	subResRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subResRange.setLayerCount(1);
	subResRange.setLevelCount(1);
	subResRange.setBaseMipLevel(0);
	subResRange.setBaseArrayLayer(0);
	
	vk::ImageViewCreateInfo viewInfo;
	viewInfo.setImage(this->handle);
	viewInfo.setFormat(format);
	viewInfo.setViewType(vk::ImageViewType::e2D);
	viewInfo.setComponents(components);
	viewInfo.setSubresourceRange(subResRange);
	view = device.createImageView(viewInfo);
}

Image::~Image()
{
	if (isMapped)
	{
		device.unmapMemory(memory);
	}
	if (selfManaged)
	{
		device.destroyImage(handle);
		device.freeMemory(memory);
	}
	if (view)
		device.destroyImageView(view);
    if (sampler)
        device.destroySampler(sampler);
}

const vk::Extent2D Image::getExtent2D() const 
{
	return vk::Extent2D{extent.width, extent.height};
}

const vk::ImageView& Image::getView() const
{
	return view;
}

const vk::Sampler& Image::getSampler() const
{
	return sampler;
}

vk::Image& Image::getImage()
{
    return handle;
}


}; // namespace render

}; // namespace sword


