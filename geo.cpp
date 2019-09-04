#include "geo.hpp"
#include <iostream>

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
	descriptions[0].format = vk::Format::eR32G32B32Sfloat;
	descriptions[0].offset = offsetof(Point, pos); //where is this macro from?
	descriptions[1].binding = 0;
	descriptions[1].location = 1;
	descriptions[1].format = vk::Format::eR32G32B32Sfloat;
	descriptions[1].offset = offsetof(Point, color); //where is this macro from?
	return descriptions;
}

void Geo::createPoint(float x, float y)
{
	points.push_back({
			glm::vec3(x, y, 0.0), 
			glm::vec3(1.0, 1.0, 1.0)});
}

Triangle::Triangle(
		glm::vec3 pos1,
		glm::vec3 pos2,
		glm::vec3 pos3)
{
	points.resize(3);
	points[0].pos = pos1;
	points[1].pos = pos2;
	points[2].pos = pos3;
}

void Geo::printPoints()
{
	for (auto point : points) {
		std::cout << "Point pos: " << point.pos.x << " " << point.pos.y << std::endl;
	}
}
