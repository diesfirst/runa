#include "sculpter.hpp"
#include <cstring>

Sculpter::Sculpter()
{
}

Sculpter::~Sculpter()
{
}

void Sculpter::createGeo()
{
	geos.push_back(Geo());
	curGeo = geos.size() - 1;
}

void Sculpter::createTriangle()
{
	assert(block != nullptr);
	geos.push_back(Triangle());
	curGeo = geos.size() - 1;
	std::memcpy(
			block->pHostMemory, 
			geos[curGeo].points.data(), 
			sizeof(Point) * geos[curGeo].points.size());
}

Geo& Sculpter::getCurGeo()
{
	return geos[curGeo];
}

void Sculpter::aquireBlock(MemoryManager& mm)
{
	block = mm.vertexBlock(5000); //arbitrary size for now
}

void Sculpter::addPointToGeo(float x, float y)
{
	assert(block != nullptr);
	assert(geos.size() != 0);
//	assert(geos[curGeo].points.size() != 0);
	geos[curGeo].createPoint(x, y);
	std::memcpy(
			block->pHostMemory, 
			geos[curGeo].points.data(), 
			sizeof(Point) * geos[curGeo].points.size());
}
