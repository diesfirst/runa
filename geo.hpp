#ifndef GEO_H
#define GEO_H

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include "occupant.hpp"

class Geo : public PointBased
{
public:
	Geo();
	virtual ~Geo();

private:

};

class Triangle : public Geo
{
public:
	Triangle(
			Point p0,
			Point p1,
			Point p2);
	Triangle(
			glm::vec3 = {-0.5, 0.5, 0.0},
			glm::vec3 = {0.0, -0.5, 0.0},
			glm::vec3 = {0.5, 0.5, 0.0});
	Triangle(
			glm::vec2,
			glm::vec2,
			glm::vec2);

private:

};

#endif /* GEO_H */
