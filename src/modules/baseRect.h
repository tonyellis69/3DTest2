#pragma once

#include <glm/glm.hpp>

/** Common stuff for rooms and other rectangles. */
class CBaseRect {
public:
	bool isOnBoundary(glm::vec3& p);
	glm::vec3 getVert(int i);

	glm::vec3 a;
	glm::vec3 b;
	glm::vec3 c;
	glm::vec3 d;
};