#include "camera.hpp"

Camera::Camera(int windowWidth, int windowHeight)
{
	view = glm::mat4(1.0);
//	view = glm::lookAt(
//			glm::vec3(2.0f, 2.0f, 2.0f),
//			glm::vec3(0.0f, 0.0f, 0.0f),
//			glm::vec3(0.0f, 0.0f, 1.0f));
	projection = glm::perspective(
			glm::radians(45.0f),
			windowWidth / (float) windowHeight,
			0.1f, 10.0f);
	projection[1][1] *= -1; //must flip y
}

Camera::~Camera()
{
}
