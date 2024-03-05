#pragma once

#include <glm.hpp>

class Camera
{
public:
	glm::vec3 position = glm::vec3(0, 2, 5);
	glm::vec2 rotation = glm::vec2(0, 0);

	void move(float angle, float fac);

	glm::mat4 getMat();
};

extern Camera camera;
