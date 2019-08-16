#include "geo.hpp"

Geo::Geo()
{
}

Geo::~Geo()
{
}

vk::VertexInputBindingDescription Geo::getBindingDescription()
{
	vk::VertexInputBindingDescription description;
	description.setBinding(0);
	description.setStride(sizeof(Point));
	description.setInputRate(vk::VertexInputRate::eVertex);
	return description;
}
	
std::array<vk::VertexInputAttributeDescription, 2> 
	Geo::getAttributeDescriptions()
{
	std::array<vk::VertexInputAttributeDescription, 2> descriptions;
	descriptions[0].binding = 0;
	descriptions[0].location = 0;
	descriptions[0].format = vk::Format::eR32G32Sfloat;
	descriptions[0].offset = offsetof(Point, pos); //where is this macro from?
	descriptions[1].binding = 0;
	descriptions[1].location = 1;
	descriptions[1].format = vk::Format::eR32G32B32Sfloat;
	descriptions[1].offset = offsetof(Point, color); //where is this macro from?
	return descriptions;
}

Triangle::Triangle(
		glm::vec2 pos1,
		glm::vec2 pos2,
		glm::vec2 pos3)
{
	points.resize(3);
	points[0].pos = pos1;
	points[1].pos = pos2;
	points[2].pos = pos3;
}
