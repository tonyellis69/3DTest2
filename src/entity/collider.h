#pragma once

#include <tuple>
#include <glm/glm.hpp>

#include "component.h"

class ColliderCmp : public CEntityCmp {
public:
	ColliderCmp(CEntity* parent) : CEntityCmp(parent) {}
	std::tuple<bool, glm::vec3> segCollisionCheck(glm::vec3& segA, glm::vec3& segB);
	std::tuple<bool, glm::vec3> entCollisionCheck(CEntity* ent2);


	float boundingRadius;
};