#include "geo.hpp"
#include <iostream>

Geo::Geo()
{
}

Geo::~Geo()
{
}

Triangle::Triangle(Point p0, Point p1, Point p2)
{
	points.push_back(p0);
	points.push_back(p1);
	points.push_back(p2);
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

Triangle::Triangle(
		glm::vec2 pos1,
		glm::vec2 pos2,
		glm::vec2 pos3)
{
	points[0].pos = glm::vec3(pos1, 0.0);
	points[1].pos = glm::vec3(pos2, 0.0);
	points[2].pos = glm::vec3(pos3, 0.0);
}
