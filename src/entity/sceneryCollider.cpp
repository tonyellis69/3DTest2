#include "sceneryCollider.h"

#include "entity.h"


std::tuple<bool, glm::vec3> CSceneryColliderCmp::segCollisionCheck(glm::vec3& segA, glm::vec3& segB) {
	if (parentEntity->modelCmp->model.BBcollision(segA, segB))
		return { true, glm::vec3() };

	return { false, glm::vec3() };
}

std::tuple<bool, glm::vec3> CSceneryColliderCmp::entCollisionCheck(CEntity* e2) {
	return { 0, {0,0,0} };
}