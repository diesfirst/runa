#include "description.hpp"
#include <cstring>
#include <bitset>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Description::Description(Context& context) :
	context(context)
{
	//arbitrary size for now...
	//could be more performant to wrap the index and vertex buffer
	//into a single uber buffer
	//one descriptor set for world transform
}

Description::~Description()
{
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

std::vector<vk::VertexInputBindingDescription> Description::getBindingDescriptions()
{
	std::vector<vk::VertexInputBindingDescription> vec;
	vk::VertexInputBindingDescription description;
	description.setBinding(0);
	description.setStride(sizeof(Point));
	description.setInputRate(vk::VertexInputRate::eVertex);
	vec.push_back(description);
	return vec;
}
	
std::vector<vk::VertexInputAttributeDescription> 
	Description::getAttributeDescriptions()
{
	std::vector<vk::VertexInputAttributeDescription> descriptions;
	descriptions.resize(2);
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

