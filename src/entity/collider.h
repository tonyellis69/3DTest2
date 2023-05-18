#pragma once

#include <tuple>
#include <glm/glm.hpp>

#include "component.h"

enum TCollider {rigidBodyCollider, missileCollider, sceneryCollider};

class ColliderCmp : public CDerivedC<ColliderCmp> {
public:
	ColliderCmp(CEntity* parent) : CDerivedC(parent) {}
	virtual std::tuple<bool, glm::vec3> segCollisionCheck(glm::vec3& segA, glm::vec3& segB);
	virtual std::tuple<bool, glm::vec3> entCollisionCheck(CEntity* ent2);
	virtual std::tuple<float, glm::vec3> sceneryCollisionCheck(CEntity* entity, int dir) {
		return { 0, {0,0,0} };
	}



	float boundingRadius;
	bool sceneryOnly = false;

	TCollider colliderType = rigidBodyCollider;
};