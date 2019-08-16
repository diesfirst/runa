#ifndef GEO_H
#define GEO_H

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>

struct Point
{
	glm::vec2 pos;
	glm::vec3 color = {0.5, 1.0, 0.6};
};

class Geo
{
public:
	Geo();
	virtual ~Geo();

	int aqcuireBufferBlock(size_t size);

	static vk::VertexInputBindingDescription getBindingDescription();
	static std::array<vk::VertexInputAttributeDescription, 2> 
		getAttributeDescriptions();

	std::vector<Point> points;
	void printPoints();

private:

};

class Triangle : public Geo
{
public:
	Triangle(
			glm::vec2 = {-0.5, 0.5},
			glm::vec2 = {0.0, -0.5},
			glm::vec2 = {0.5, 0.5});

private:

};

#endif /* GEO_H */
