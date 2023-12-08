#pragma once

#include <glm/glm.hpp>

/** Common stuff for rooms and other rectangles. */
class CBaseRect {
public:
	bool isOnBoundary(glm::vec3& p);
	glm::vec3 getVert(int i);
	void setVerts(glm::vec3&v, glm::vec3& op);
	bool operator ==(const CBaseRect& other);
	bool overlap(CBaseRect& other);
	float volume();
	float width();
	float height();

	glm::vec3 a;
	glm::vec3 b;
	glm::vec3 c;
	glm::vec3 d;
};