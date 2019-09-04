#ifndef SCULPTER_H
#define SCULPTER_H

#include "geo.hpp"
#include "mem.hpp"

class Sculpter
{
public:
	Sculpter();
	virtual ~Sculpter();

	void createGeo();
	void createTriangle();
	void aquireBlock(MemoryManager& mm);
	void addPointToGeo(float x, float y);

	Geo& getCurGeo();
	bufferBlock* block;

private:
	uint32_t curGeo = 0;
	std::vector<Geo> geos;
	
};

#endif /* SCULPTER_H */
