#include "occupant.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Occupant::Occupant()
{
}

Occupant::~Occupant()
{
}

Image::Image(const Context& context, std::string imageFilePath)
{

}

Image::Image(const Context* context, uint16_t w, uint16_t h, uint8_t channelCount)
{
	width = w;
	height = h;
	pContext = context;
	layout = vk::ImageLayout::eUndefined;
	imageSize = w * h * channelCount;
	bufferBlock = context->getStagingBuffer(imageSize);
	imageBlock = context->getImageBlock(w, h);
}
//currently always mapped to host memory

Image::~Image()
{	
}


void Image::loadArray(unsigned char* array, size_t size)
{
	assert(bufferBlock != nullptr);
	memcpy(bufferBlock->pHostMemory, array, size);
	transitionLayoutTo(vk::ImageLayout::eTransferDstOptimal);
	pContext->pCommander->copyBufferToImageSingleTime(
			bufferBlock->buffer,
			imageBlock->image,
			width, height, 0);
}

void Image::transitionLayoutTo(vk::ImageLayout newLayout)
{
	assert(pContext != nullptr);
	pContext->pCommander->transitionImageLayout(
			imageBlock->image,
			layout,
			newLayout);
	layout = newLayout;
}

Transformable::Transformable()
{
	transform = glm::mat4(1.0); //identity
}

Transformable::~Transformable()
{
}

glm::mat4& Transformable::getTransform()
{
	return transform;
}

void Transformable::setTransform(glm::mat4 transform)
{
	this->transform = transform;
}

void Transformable::scale(float scale)
{
	setTransform(glm::scale(transform, glm::vec3(scale,scale,scale)));
}

void Transformable::translate(float x, float y, float z)
{
	setTransform(glm::translate(transform, glm::vec3(x, y, z)));
}

void Transformable::rotate(float angle, glm::vec3 axis)
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

Point* PointBased::getPoint(uint32_t index)
{
	if (index > points.size())
	{
		std::cout << "Index: " << index << std::endl;
		std::cout << "Cannot get point, index out of range. Number of points: " 
			<< points.size() << std::endl;
		return nullptr;
	}
	return &points[index];
}

void PointBased::printPoints() const
{
	for (auto point : points) {
		std::cout << "Point pos: " << point.pos.x << " " << point.pos.y << std::endl;
	}
}

void PointBased::setVertOffset(uint32_t offset)
{
	vertOffset = offset;
}

u_int32_t PointBased::getVertOffset() const
{
	return vertOffset;
}

Geo::Geo()
{
}

Geo::~Geo()
{
}

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

Triangle::Triangle(Point p0, Point p1, Point p2)
{
	points.push_back(p0);
	points.push_back(p1);
	points.push_back(p2);
	indices = {0, 1, 2};
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
	indices = {0, 1, 2};
}

Triangle::Triangle(
		glm::vec2 pos1,
		glm::vec2 pos2,
		glm::vec2 pos3)
{
	points.resize(3);
	points[0].pos = glm::vec3(pos1, 0.0);
	points[1].pos = glm::vec3(pos2, 0.0);
	points[2].pos = glm::vec3(pos3, 0.0);
	indices = {0, 1, 2};
}

Quad::Quad(
		glm::vec3 pos0,
		glm::vec3 pos1,
		glm::vec3 pos2,
		glm::vec3 pos3)
{
	points.resize(4);
	points[0].pos = pos0;
	points[1].pos = pos1;
	points[2].pos = pos2;
	points[3].pos = pos3;
	indices = {0, 1, 2, 2, 3, 0};
}

Quad::~Quad()
{
}

Camera::Camera(int windowWidth, int windowHeight)
{
	view = glm::mat4(1.0);
//	view = glm::lookAt(
//			glm::vec3(2.0f, 2.0f, 2.0f),
//			glm::vec3(0.0f, 0.0f, 0.0f),
//			glm::vec3(0.0f, 0.0f, 1.0f));
	projection = glm::perspective(
			glm::radians(45.0f),
			windowWidth / (float) windowHeight,
			0.1f, 10.0f);
	projection[1][1] *= -1; //must flip y
}

Camera::~Camera()
{
}
