#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include <vector>
#include "context.hpp"
#include "mem.hpp"
#include "occupant.hpp"

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

struct DrawableInfo
{
	uint32_t indexBufferOffset;
	uint32_t numberOfIndices;
	uint32_t vertexIndexOffset;
};

class Description
{
public:
	Description(Context&);
	~Description();
	void createCamera(const uint16_t width, const uint16_t height);

	void loadImage(std::string path);
	std::vector<vk::DescriptorSet> descriptorSets;
	vk::Buffer& getVkVertexBuffer();
	vk::Buffer& getVkIndexBuffer();
	uint32_t getVertexCount();
	uint32_t getMeshCount();
	uint32_t getOccupantCount();
	uint32_t getDynamicAlignment();
	std::vector<DrawableInfo>& getDrawInfos();
	vk::DescriptorSetLayout* getPDescriptorSetLayout();
	void prepareDescriptorSets(const uint32_t count);
	void prepareUniformBuffers(const uint32_t count);
	void setCurrentSwapIndex(uint8_t swapIndex);
	void updateAllCurrentUbos();
	void update(PointBased*);

	static vk::VertexInputBindingDescription getBindingDescription();
	static std::array<vk::VertexInputAttributeDescription, 2> 
		getAttributeDescriptions();

	template<typename T> T* createMesh()
	{
		auto mesh = std::make_shared<T>();
		meshes.push_back(mesh);
		pointBasedOccupants.push_back(mesh);
		occupants.push_back(mesh);
		mesh->setVertOffset(curVertOffset);
		updateIndexBuffer(*mesh);
		updateVertexBuffer(*mesh);
		return mesh.get();
	}

private:
	Context& context;
	std::vector<std::shared_ptr<Occupant>> occupants;
	std::vector<std::shared_ptr<PointBased>> pointBasedOccupants;
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<Camera>> cameras;
	std::vector<DrawableInfo> drawInfos;
	std::shared_ptr<Camera> curCamera;
	BufferBlock* vertexBlock; //does not have ownership
	BufferBlock* indexBlock; //does not have ownership
	std::vector<BufferBlock*> uboBlocks;
	std::vector<BufferBlock*> uboDynamicBlocks;
	uint8_t curSwapIndex = 0;
	UboVS uboView;
	UboDynamic uboDynamicData;
	size_t dynamicAlignment;
	std::unique_ptr<Image> texture1;

	Point* vertices;
	uint32_t* indices;
	uint32_t curVertOffset = 0; 
	uint32_t curIndexOffset = 0;

	vk::DescriptorSetLayout descriptorSetLayout;
	uint32_t descriptorSetCount;
	bool descriptorsPrepared = false;
	vk::DescriptorPool descriptorPool;

	void updateCommandBuffer();
	void oldUpdateVertexBuffer();
	void updateVertexBuffer(const PointBased&);
	void updateIndexBuffer(const Mesh&);
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
