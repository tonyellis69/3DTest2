#pragma once

#include <tuple>
#include <vector>
#include <glm/glm.hpp>

#include "collider.h"


struct TMissileCollision {
	CEntity* ent;
	float dist;
	glm::vec3 collisionPt;
};

class CMissileColliderCmp : public ColliderCmp {
public:
	CMissileColliderCmp(CEntity* parent) : ColliderCmp(parent) {
		sceneryOnly = false;
		colliderType = missileCollider;
	}
	void onSpawn();
	//std::tuple<bool, glm::vec3> segCollisionCheck(glm::vec3& segA, glm::vec3& segB) ;
	std::tuple<bool, glm::vec3> entCollisionCheck(CEntity* ent2);
	//std::tuple<float, glm::vec3> sceneryCollisionCheck(CEntity* entity, int dir);

	void update(float dT);


	float distToPoint = 0.1f;
	glm::vec3 startingPos;

	std::vector<TMissileCollision> collisions;
};