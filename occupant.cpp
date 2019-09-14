#include "occupant.hpp"

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
