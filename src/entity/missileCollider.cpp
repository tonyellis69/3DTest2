#pragma once

#include <tuple>
#include <algorithm>
#include <glm/glm.hpp>

#include "missileCollider.h"
#include "entity.h"

#include "../explosion.h"
#include "../spawner.h"


/** Record any collision with this entity, but return none as we don't want the
	default physics resolution.*/
std::tuple<bool, glm::vec3> CMissileColliderCmp::entCollisionCheck(CEntity* ent2) {
	if (ent2->id == thisEntity->getParent()->id)
		return { 0, {0,0,0} };

	glm::vec3 segStart = startingPos;// thisEntity->getPos();
	glm::vec3 segEnd = thisEntity->getPos() + (thisEntity->transform->getRotationVec() * distToPoint);
	if (ent2->modelCmp->model.BBcollision(segStart, segEnd)) {
		collisions.push_back({ ent2, glm::distance(segStart, ent2->getPos()), ent2->getPos() });
	}

	return { 0, {0,0,0} };
}

void CMissileColliderCmp::onSpawn()
{
	startingPos = thisEntity->getPos();
}

void CMissileColliderCmp::update(float dT) {
	if (!collisions.empty()) {
		std::sort(collisions.begin(), collisions.end(),
			[](TMissileCollision& A, TMissileCollision& B) { return A.dist < B.dist; } );
		
		auto collision = collisions.begin();
		CExplosion* splode = (CExplosion*)spawn::explosion("explosion", collision->collisionPt, 1).get();
		if (collision->ent) {
			splode->setCollidee(collision->ent);
			collision->ent->receiveDamage(*thisEntity->getParent(), 5);
			//FIXME: need to pass on missile-firer, which entity system does not currently allow

		}
		thisEntity->destroyMe();
	}


}
