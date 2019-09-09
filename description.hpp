#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include "geo.hpp"
#include "camera.hpp"
#include <vector>

class Description
{
public:
	Description(Context&);
	~Description();
	void createCamera();
	void createGeo();

private:
	Context& context;
	std::vector<Camera> cameras;
	std::vector<Geo> geometry;
	std::vector<vk::DescriptorSet> descriptorSets;
};

#endif /* DESCRIPTION_H */
