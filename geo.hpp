#ifndef GEO_H
#define GEO_H

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>

struct Point
{
	glm::vec3 pos;
	glm::vec3 color = {0.3, 0.7, 0.8};
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

	void createPoint(float x, float y);
	void createPoint(float x, float y, float z);

	std::vector<Point> points;
	void printPoints();

private:

};

class Triangle : public Geo
{
public:
	Triangle(
			glm::vec3 = {-0.1, 0.5, 0.0},
			glm::vec3 = {0.0, -0.5, 0.0},
			glm::vec3 = {0.5, 0.5, 0.0});

private:

};

#endif /* GEO_H */
