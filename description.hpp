#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include <vector>
#include "context.hpp"
#include "mem.hpp"

class Viewport; //not a strong dependency
class Geo;
class Camera;

class Description
{
public:
	Description(Context&);
	~Description();
	void bindViewport(Viewport); //must be called for now
	void createCamera();
	void createGeo();

private:
	Context& context;
	std::vector<Camera> cameras;
	std::vector<Geo> geometry;
	std::vector<vk::DescriptorSet> descriptorSets;
	vk::DescriptorSetLayout descriptorSetLayout;
	uint32_t descriptorCount;
	bool descriptorsPrepared = false;
	vk::DescriptorPool descriptorPool;
	Viewport* viewport; //beware raw pointer

	void prepareDescriptorSets(uint32_t count);
	void initDescriptorSetLayout();
	void createDescriptorPool(uint32_t descriptorCount);
	void createDescriptorSets(uint32_t descriptorCount);
	void updateDescriptorSets(
			uint32_t descriptorCount, 
			std::vector<bufferBlock>* uboBlocks,
			size_t uboSize);
};

#endif /* DESCRIPTION_H */
