#include "missile.h"

#include <glm/gtc/matrix_transform.hpp>

#include "gameState.h"
#include "utils/log.h"

//#include "hexRenderer.h"

#include "explosion.h"

#include "spawner.h"

CMissile::CMissile() {
	entityType = entMissile;
}

void CMissile::setPosition(glm::vec3& pos, float rotation) {
	//CSprite::setPosition(pos, rotation);

	//worldPos = pos;
	transform->setPos(pos);
	transform->setRotation(rotation);

	//modelCmp->translateAll(getPos());
	//modelCmp->rotate(transform->rotation);

	//dirVec =  glm::normalize(modelCmp->model.getMainMesh()->matrix * glm::vec4(1, 0, 0, 0));
	dirVec =  { cos(rotation), -sin(rotation),0 }; 

	leadingPoint = pos + dirVec * distToPoint;
	leadingPointLastHex = leadingPoint;
	lastLeadingPointHex = worldSpaceToHex(pos);
	startingPos = pos;
}

void CMissile::update(float dT) {
	CEntity::update(dT);
	if (collided) {
		spawnExplosion();
		destroyMe();
		return;
	}
	this->dT = dT;
	approachDestHex();
}

void CMissile::draw() {
	//hexRendr2.drawLineModel(model.meshes[0]);
}

void CMissile::setOwner(CEntity* owner) {
	this->owner = owner;
}

void CMissile::setSpeed(float speed) {
	missileMoveSpeed = speed;
}


/** Move realtime in the current  direction. */
void CMissile::approachDestHex() {
	glm::vec3 moveVec = dirVec * missileMoveSpeed * 1000.0f;

	phys->moveImpulse = moveVec ;



//	glm::vec3 currentWPos = transform->worldPos;
//	transform->setPos(getPos() + moveVec);


	leadingPoint = getPos() + dirVec * distToPoint;

	collisionCheck(moveVec);

}

/** Check for a collision along the line segment from the leading point's last known
	position to where it is now. */
bool CMissile::collisionCheck(glm::vec3& moveVec) 
{	
	return true; //
	//first, collect any new unique hexes that we've intersected
	CHex leadingPointHex = worldSpaceToHex(leadingPoint);
	if (leadingPointHex != lastLeadingPointHex) { //we've moved at least one hex on
		intersectedHexes = getIntersectedHexes(leadingPointLastHex, leadingPoint);
	}

	//Check if we've collided with an entity in one of those hexes
	for (auto& hex: intersectedHexes) {
		CEntities entities = game.getEntitiesAt(hex.first);
		for (auto& entity : entities) {
			if (entity && entity != this && entity != owner && entity->live) {
				if (entity->collider && !entity->collider->sceneryOnly) {
					auto [hit, intersection] = entity->collider->segCollisionCheck(startingPos, leadingPoint);
					if (hit) {
						return true;
						collisionPt = entity->getPos();
						collidee = std::make_shared<CEntity>(*entity);
						entity->healthC->receiveDamage(*owner, 5);
						collided = true;
						return true;
					}
				}
			}
		}
	}

	//still here? Check if we've collided with scenery - but only if we've entered a new hex.
	if (leadingPointHex != lastLeadingPointHex) {
		for (auto& hex : intersectedHexes) {
			if (game.level->getHexArray()->getHexCube(hex.first).content == solidHex) {
				collided = true;
				//transform->worldPos = hex.second - (moveVec * distToPoint);
				transform->setPos( hex.second - (moveVec * distToPoint));
				collisionPt = hex.second;
				return true;
			}
		}
	
		//optimisation: remove oldest hex from list if safe to do so
		if (intersectedHexes.size() > 1) {
			//auto exitedHex = intersectedHexes.find(lastLeadingPointHex);
			auto exitedHex = intersectedHexes.begin();
			if (exitedHex  != intersectedHexes.end())
				intersectedHexes.erase(exitedHex);
		}
		lastLeadingPointHex = leadingPointHex;
	}

	return false;
}

void CMissile::spawnExplosion() {
	CExplosion* splode = (CExplosion *) spawn::explosion("explosion", collisionPt, 1);
	if (collidee)
		splode->setCollidee(collidee.get());

}


