#ifndef CAMERA_H
#define CAMERA_H value
#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "mem.hpp"

struct ViewTransforms
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class Camera
{
public:
	Camera (int w, int h);
	virtual ~Camera ();
	void acquireUniformBufferBlocks(MemoryManager&, int count);
	void updateModelXForm(float radians);
	void copyXFormsToBuffer(int index);
	void copyXFormsToBuffers();
	std::vector<bufferBlock>* uboBlocks;

private:
	ViewTransforms xforms;
	uint32_t blockCount;

	/* data */
};

#endif /* ifndef CAMERA_H */
