#ifndef OCCUPANT_H
#define OCCUPANT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include "context.hpp"

class Occupant
{
public:
	Occupant();
	virtual ~Occupant();

private:
};

class Image : public Occupant
{
public:
	Image(const Context& context, std::string imageFilePath);
	Image(const Context* context, uint16_t width, uint16_t height, uint8_t channels);
	void transitionLayoutTo(vk::ImageLayout);
	void loadArray(unsigned char* array, size_t size);
	vk::Image* getPVKImage() const;
	virtual ~Image();
	uint16_t width;
	uint16_t height;
private:
	const Context* pContext;
	vk::DeviceSize imageSize;
	vk::ImageLayout layout;
	BufferBlock* bufferBlock;
	ImageBlock* imageBlock;
};

class Transformable : public Occupant
{
public:
	Transformable();
	virtual ~Transformable();

	glm::mat4& getTransform();
	void setTransform(glm::mat4 transform);
	void scale(float scale);
	void translate(float x, float y, float z);
	void rotate(float angle, glm::vec3 axis);

	glm::mat4 transform;

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
	void setColor(float r, float g, float b)
	{
		color = {r, g, b};
	}
};

class PointBased : public Transformable
{
public:
	PointBased();
	virtual ~PointBased();

	void createPoint(float x, float y);
	void createPoint(float x, float y, float z);
	Point* getPoint(uint32_t i);
	void printPoints() const;
	void setVertOffset(uint32_t);
	uint32_t getVertOffset() const;

	std::vector<Point> points;
private:
	u_int32_t vertOffset = 0;

};

class Geo : public PointBased
{
public:
	Geo();
	virtual ~Geo();

private:

};

class Mesh : public Geo
{
public:
	Mesh ();
	virtual ~Mesh ();
	std::vector<uint32_t> indices;

private:
};

class Triangle : public Mesh
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

class Quad : public Mesh
{
public:
	Quad(
			glm::vec3 = {-0.5, -0.5, 0.0}, //top left
			glm::vec3 = {0.5, -0.5, 0.0}, //top right
			glm::vec3 = {0.5, 0.5, 0.0}, //bottom right
			glm::vec3 = {-0.5, 0.5, 0.0}); //bottom left
	virtual ~Quad ();

private:
	/* data */
};

class Camera : public Transformable
{
public:
	Camera (int w, int h);
	virtual ~Camera ();

	glm::mat4 projection;
	glm::mat4 view;

private:

	/* data */
};

#endif /* OCCUPANT_H */
