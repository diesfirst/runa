#include "occupant.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Occupant::Occupant()
{
	transform = glm::mat4(1.0); //identity
}

Occupant::~Occupant()
{
}

glm::mat4& Occupant::getTransform()
{
	return transform;
}

void Occupant::setTransform(glm::mat4 transform)
{
	this->transform = transform;
}

void Occupant::scale(float scale)
{
	setTransform(glm::scale(transform, glm::vec3(scale,scale,scale)));
}

void Occupant::translate(float x, float y, float z)
{
	setTransform(glm::translate(transform, glm::vec3(x, y, z)));
}

void Occupant::rotate(float angle, glm::vec3 axis)
{
	setTransform(glm::rotate(transform, angle, axis));
}

PointBased::PointBased()
{
}

PointBased::~PointBased()
{
}

void PointBased::createPoint(float x, float y)
{
	points.push_back({
			glm::vec3(x, y, 0.0), 
			glm::vec3(1.0, 1.0, 1.0)});
}

void PointBased::createPoint(float x, float y, float z)
{
	points.push_back({
			glm::vec3(x, y, z), 
			glm::vec3(1.0, 1.0, 1.0)});
}

void PointBased::printPoints()
{
	for (auto point : points) {
		std::cout << "Point pos: " << point.pos.x << " " << point.pos.y << std::endl;
	}
}
