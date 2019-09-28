#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include <vector>
#include "context.hpp"
#include "mem.hpp"
#include "geo.hpp"
#include "camera.hpp"

class Viewport; //not a strong dependency

class Description
{
public:
	Description(Context&);
	~Description();
	void createCamera();
	void createGeo();
	void createTriangle();
	void createTriangle(Point, Point, Point);
	std::vector<vk::DescriptorSet> descriptorSets;
	vk::Buffer& getVkVertexBuffer();
	uint32_t getVertexCount();
	vk::DescriptorSetLayout* getPDescriptorSetLayout();

	static vk::VertexInputBindingDescription getBindingDescription();
	static std::array<vk::VertexInputAttributeDescription, 2> 
		getAttributeDescriptions();

private:
	Context& context;
	std::vector<std::shared_ptr<Occupant>> occupants;
	std::vector<std::shared_ptr<PointBased>> pointBasedOccupants;
	std::vector<std::shared_ptr<Geo>> geometry;
	std::vector<std::shared_ptr<Camera>> cameras;
	BufferBlock* vertexBlock; //does not have ownership
	Point* vertices;
	vk::DescriptorSetLayout descriptorSetLayout;
	uint32_t descriptorCount;
	bool descriptorsPrepared = false;
	vk::DescriptorPool descriptorPool;

	void updateCommandBuffer();
	void updateVertexBuffer();
	void prepareDescriptorSets(uint32_t count);
	void initDescriptorSetLayout();
	void createDescriptorPool(uint32_t descriptorCount);
	void createDescriptorSets(uint32_t descriptorCount);
	void updateDescriptorSets(
			uint32_t descriptorCount, 
			std::vector<BufferBlock>* uboBlocks,
			size_t uboSize);
};

#endif /* DESCRIPTION_H */
