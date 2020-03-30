#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include <vector>
#include "context.hpp"
#include "mem.hpp"
#include <occupant.hpp>

constexpr size_t MAX_OCCUPANTS = 100;

class Description
{
public:
	Description(Context&);
	~Description();
	void createCamera(const uint16_t width, const uint16_t height);

	std::vector<vk::DescriptorSet> descriptorSets;
	vk::Buffer& getVkVertexBuffer();
	vk::Buffer& getVkIndexBuffer();
	uint32_t getVertexCount();
	uint32_t getMeshCount();
	uint32_t getOccupantCount();
	uint32_t getDynamicAlignment();
	vk::DescriptorSetLayout* getPDescriptorSetLayout();
	void prepareDescriptorSets(const uint32_t count);
	void prepareUniformBuffers(const uint32_t count);
	void updateAllCurrentUbos(uint32_t swapIndex);
	void update(PointBased*);

	static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions();
	static std::vector<vk::VertexInputAttributeDescription> 
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
	std::shared_ptr<Camera> curCamera;
	size_t dynamicAlignment;

	Point* vertices;
	uint32_t* indices;
	uint32_t curVertOffset = 0; 
	uint32_t curIndexOffset = 0;

};

#endif /* DESCRIPTION_H */
