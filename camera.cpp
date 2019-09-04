#include "camera.hpp"
#include <memory>

Camera::Camera(int windowWidth, int windowHeight)
{
	xforms.model = glm::mat4(1.0f);
	xforms.view = glm::lookAt(
			glm::vec3(2.0f, 2.0f, 2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
	xforms.proj = glm::perspective(
			glm::radians(45.0f),
			windowWidth / (float) windowHeight,
			0.1f, 10.0f);
	xforms.proj[1][1] *= -1; //must flip y
}

Camera::~Camera()
{
}

void Camera::updateModelXForm(float theta)
{
	xforms.model = glm::rotate(
			glm::mat4(1.0f), 
			theta * glm::radians(90.0f),
			glm::vec3(0, 0, 1));
}

void Camera::copyXFormsToBuffer(int index)
{
	std::memcpy((*uboBlocks)[index].pHostMemory, &xforms, sizeof(xforms));
}

void Camera::copyXFormsToBuffers()
{
	for (int i = 0; i < blockCount; ++i) {
		copyXFormsToBuffer(i);
	}
}

void Camera::acquireUniformBufferBlocks(MemoryManager& mm, int count)
{
	mm.createUniformBuffers(count, sizeof(ViewTransforms));
	uboBlocks = &mm.uniformBufferBlocks;
	blockCount = count;
}
