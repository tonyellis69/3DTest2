#pragma once

#include <tuple>
#include <glm/glm.hpp>

#include "collider.h"

class CSceneryColliderCmp : public ColliderCmp {
public:
	CSceneryColliderCmp(CEntity* parent) : ColliderCmp(parent) {
		sceneryOnly = true;
	}
	std::tuple<bool, glm::vec3> segCollisionCheck(glm::vec3& segA, glm::vec3& segB);
	std::tuple<bool, glm::vec3> entCollisionCheck(CEntity* ent2);

};