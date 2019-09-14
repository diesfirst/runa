#ifndef OCCUPANT_H
#define OCCUPANT_H

#include <glm/glm.hpp>

class Occupant
{
public:
	Occupant();
	virtual ~Occupant();
	glm::mat4& getTransform();

	glm::mat4 transform;

private:
	

};

#endif /* OCCUPANT_H */
