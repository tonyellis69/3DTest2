#pragma once

#include <tuple>
#include <algorithm>
#include <glm/glm.hpp>

#include "missileCollider.h"
#include "entity.h"

#include "../explosion.h"
#include "../spawner.h"

#include "../gameState.h"

/** Record any collision with this entity, but return none as we don't want the
	default physics resolution.*/
std::tuple<bool, glm::vec3> CMissileColliderCmp::entCollisionCheck(CEntity* ent2) {
	if (ent2->id == thisEntity->getParent()->id)
		return { 0, {0,0,0} };

	glm::vec3 segStart = startingPos;// thisEntity->getPos();
	glm::vec3 segEnd = leadingPoint;
	if (ent2->modelCmp->model.BBcollision(segStart, segEnd)) {
		collisions.push_back({ ent2, glm::distance(segStart, ent2->getPos()), ent2->getPos() });
	}

	return { 0, {0,0,0} };
}

void CMissileColliderCmp::onSpawn()
{
	startingPos = thisEntity->getPos();
	newHex = CHex(-1);
	leadingPoint = thisEntity->getPos() + (thisEntity->transform->getRotationVec() * distToPoint);
}

void CMissileColliderCmp::update(float dT) {
	oldLeadingPoint = leadingPoint;
	leadingPoint = thisEntity->getPos() + (thisEntity->transform->getRotationVec() * distToPoint);


	if (!collisions.empty()) {
		std::sort(collisions.begin(), collisions.end(),
			[](TMissileCollision& A, TMissileCollision& B) { return A.dist < B.dist; } );
		
		auto collision = collisions.begin();
		CExplosion* splode = (CExplosion*)spawn::explosion("explosion", collision->collisionPt, 1);
		if (collision->ent) {
			splode->setCollidee(collision->ent);
			if (collision->ent->healthC)
				collision->ent->healthC->receiveDamage(*thisEntity->getParent(), 5);
		}
		thisEntity->destroyMe();
		return;
	}


	//still here? Maybe check for scenery collisions.
	CHex leadingPointHex = worldSpaceToHex(leadingPoint);
	if (leadingPointHex != newHex) {
		newHex = leadingPointHex;

		//now check for collisions along this path. 
		auto [impact, collisionPt] = sceneryCollisionCheck(oldLeadingPoint, leadingPoint);
		if (impact) {
			CExplosion* splode = (CExplosion*)spawn::explosion("explosion", collisionPt, 1);
			thisEntity->destroyMe();
			return;
		}
	}


}

std::tuple<bool, glm::vec3> CMissileColliderCmp::sceneryCollisionCheck(glm::vec3& segA, glm::vec3& segB) {
	TIntersections intersectedHexes = getIntersectedHexes(segA, segB);
	
	for (auto& hex : intersectedHexes) {
		if (game.level->getHexArray()->getHexCube(hex.first).content == solidHex) 
			return { true, {hex.second} };
	}
	return { false, {0,0,0} };
}
