#ifndef OCCUPANT_H
#define OCCUPANT_H

#include <glm/glm.hpp>
#include <vector>
#include <iostream>

class Occupant
{
public:
	Occupant();
	virtual ~Occupant();
	glm::mat4& getTransform();

	glm::mat4 transform;

private:
};

struct Point
{
	glm::vec3 pos;
	glm::vec3 color = {0.3, 0.7, 0.8};
	void print()
	{
		std::cout << "Point at " << this << std::endl;
		std::cout << "x: " << pos.x << std::endl;
		std::cout << "y: " << pos.y << std::endl;
		std::cout << "z: " << pos.z << std::endl;
	}
};

class PointBased : public Occupant
{
public:
	PointBased();
	virtual ~PointBased();

	void createPoint(float x, float y);
	void createPoint(float x, float y, float z);
	void printPoints();

	std::vector<Point> points;

};

#endif /* OCCUPANT_H */
