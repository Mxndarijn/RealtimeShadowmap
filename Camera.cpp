#include "Camera.h"
#include <gtc/matrix_transform.hpp>

Camera camera;

void Camera::move(float angle, float fac)
{
	position.x -= (float)cos((rotation.x + angle) / 180 * glm::pi<float>()) * fac;
	position.z -= (float)sin((rotation.x + angle) / 180 * glm::pi<float>()) * fac;
}

glm::mat4 Camera::getMat()
{
	glm::mat4 mat = glm::mat4(1);// glm::lookAt(glm::vec3(0, 10, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(1, 0, 0));
	mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(0, 1, 0));
	mat = glm::translate(mat, -position);
	return mat;
}
