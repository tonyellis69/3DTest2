#pragma once

#include <tuple>
#include <glm/glm.hpp>

#include "component.h"

class ColliderCmp : public CEntityCmp {
public:
	ColliderCmp(CEntity* parent) : CEntityCmp(parent) {}
	virtual std::tuple<bool, glm::vec3> segCollisionCheck(glm::vec3& segA, glm::vec3& segB);
	virtual std::tuple<bool, glm::vec3> entCollisionCheck(CEntity* ent2);


	float boundingRadius;
	bool sceneryOnly = false;
};