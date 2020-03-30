#include <render/attachment.hpp>
#include <iostream>

namespace sword
{

namespace render
{

Attachment::Attachment(
		const vk::Device& device,
		const vk::Extent2D extent,
        const vk::ImageUsageFlags usageFlags) :
	device{device},
	extent{extent}
{
	vk::Extent3D ex = {extent.width, extent.height, 1};
	format = vk::Format::eR8G8B8A8Unorm;
	auto image = std::make_unique<Image>(
			device, 
			ex,
			format,
            usageFlags,
			vk::ImageLayout::eUndefined);
	images.emplace_back(std::move(image));
	std::cout << "IMG " << &images.at(0) << std::endl;
}

Attachment::Attachment(const vk::Device& device, std::unique_ptr<Image> swapimage) :
	device{device}
{
	extent = swapimage->getExtent2D();
	images.push_back(std::move(swapimage));
}

Attachment::~Attachment()
{
}

vk::Format Attachment::getFormat() const
{
	return format;
}

vk::Extent2D Attachment::getExtent() const
{
    return extent;
}

Image& Attachment::getImage(uint32_t index)
{
    return *images.at(index);
}

}; // namespace render

}; // namespace sword
