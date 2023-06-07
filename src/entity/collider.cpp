#include "collider.h"

#include "entity.h"


void ColliderCmp::onAdd() {
	thisEntity->collider = this;
}

void ColliderCmp::onRemove() {
	thisEntity->collider = nullptr;
}


std::tuple<bool, glm::vec3> ColliderCmp::segCollisionCheck(glm::vec3& segA, glm::vec3& segB) {
   // return thisEntity->collisionCheck(segA, segB);

	if (thisEntity->modelCmp->model.BBcollision(segA, segB))
		return { true, glm::vec3() };

	return { false, glm::vec3() };
}

std::tuple<bool, glm::vec3> ColliderCmp::entCollisionCheck(CEntity* e2) {
	//get bounding-sphere radii
	float radius1 = thisEntity->modelCmp->model.getRadius();
	float radius2 = e2->modelCmp->model.getRadius();

	//check for overlap
	float entDist = glm::distance(thisEntity->getPos(), e2->getPos());
	if (entDist > radius1 + radius2)
		return { 0, {0,0,0} };
	else {
		float collisionDist = entDist - radius2;
		glm::vec3 collisionNormal = glm::normalize(e2->getPos() - thisEntity->getPos());
		return { collisionDist, collisionNormal };
	}

}