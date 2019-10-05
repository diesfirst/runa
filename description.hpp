#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include <vector>
#include "context.hpp"
#include "mem.hpp"
#include "geo.hpp"
#include "camera.hpp"

class Viewport; //not a strong dependency

constexpr size_t MAX_OCCUPANTS = 100;

struct UboDynamic
{
	glm::mat4* model = nullptr;
};

struct UboVS
{
	glm::mat4 projection;
	glm::mat4 view;
};

class Description
{
public:
	Description(Context&);
	~Description();
	void createCamera(const uint16_t width, const uint16_t height);
	void createGeo();
	Triangle* createTriangle();
	void createTriangle(Point, Point, Point);
	std::vector<vk::DescriptorSet> descriptorSets;
	vk::Buffer& getVkVertexBuffer();
	uint32_t getVertexCount();
	uint32_t getGeoCount();
	uint32_t getOccupantCount();
	uint32_t getDynamicAlignment();
	vk::DescriptorSetLayout* getPDescriptorSetLayout();
	void prepareDescriptorSets(const uint32_t count);
	void prepareUniformBuffers(const uint32_t count);
	void setCurrentSwapIndex(uint8_t swapIndex);
	void updateAllCurrentUbos();

	static vk::VertexInputBindingDescription getBindingDescription();
	static std::array<vk::VertexInputAttributeDescription, 2> 
		getAttributeDescriptions();

private:
	Context& context;
	std::vector<std::shared_ptr<Occupant>> occupants;
	std::vector<std::shared_ptr<PointBased>> pointBasedOccupants;
	std::vector<std::shared_ptr<Geo>> geometry;
	std::vector<std::shared_ptr<Camera>> cameras;
	std::shared_ptr<Camera> curCamera;
	BufferBlock* vertexBlock; //does not have ownership
	std::vector<BufferBlock>* uboBlocks;
	std::vector<BufferBlock>* uboDynamicBlocks;
	uint8_t curSwapIndex = 0;
	UboVS uboView;
	UboDynamic uboDynamicData;
	size_t dynamicAlignment;
	Point* vertices;
	vk::DescriptorSetLayout descriptorSetLayout;
	uint32_t descriptorSetCount;
	bool descriptorsPrepared = false;
	vk::DescriptorPool descriptorPool;

	void updateCommandBuffer();
	void updateVertexBuffer();
	void updateUniformBuffers();
	void updateDynamicUniformBuffers();
	void initDescriptorSetLayout();
	void createDescriptorPool(uint32_t descriptorSetCount);
	void createDescriptorSets(uint32_t descriptorSetCount);
	void updateDescriptorSets(
			uint32_t descriptorCount, 
			std::vector<BufferBlock>* uboBlocks,
			size_t uboSize);
};

#endif /* DESCRIPTION_H */
