#ifndef CAMERA_H
#define CAMERA_H value
#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "occupant.hpp"


class Camera : public Occupant
{
public:
	Camera (int w, int h);
	virtual ~Camera ();

	glm::mat4 projection;
	glm::mat4 view;

private:

	/* data */
};

#endif /* ifndef CAMERA_H */
