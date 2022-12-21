#include "collider.h"

#include "entity.h"


std::tuple<bool, glm::vec3> ColliderCmp::segCollisionCheck(glm::vec3& segA, glm::vec3& segB) {
   // return parentEntity->collisionCheck(segA, segB);

	if (parentEntity->modelCmp->model.BBcollision(segA, segB))
		return { true, glm::vec3() };

	return { false, glm::vec3() };
}

std::tuple<bool, glm::vec3> ColliderCmp::entCollisionCheck(CEntity* e2) {
	//get bounding-sphere radii
	float radius1 = parentEntity->modelCmp->model.getRadius();
	float radius2 = e2->modelCmp->model.getRadius();

	//check for overlap
	float entDist = glm::distance(parentEntity->getPos(), e2->getPos());
	if (entDist > radius1 + radius2)
		return { 0, {0,0,0} };
	else {
		float collisionDist = entDist - radius2;
		glm::vec3 collisionNormal = glm::normalize(e2->getPos() - parentEntity->getPos());
		return { collisionDist, collisionNormal };
	}

}