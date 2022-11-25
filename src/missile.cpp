#include "missile.h"

#include <glm/gtc/matrix_transform.hpp>

#include "gameState.h"
#include "utils/log.h"

#include "hexRenderer.h"

#include "explosion.h"

#include "spawner.h"

CMissile::CMissile() {
	entityType = entMissile;
}

void CMissile::setPosition(glm::vec3& pos, float rotation) {
	//CSprite::setPosition(pos, rotation);

	worldPos = pos;
	this->setRotation(rotation);
	buildWorldMatrix();



	dirVec =  glm::normalize( model.getMainMesh()->matrix * glm::vec4(1, 0, 0, 0));
	leadingPoint = worldPos + dirVec * distToPoint;
	leadingPointLastHex = leadingPoint;
	lastLeadingPointHex = worldSpaceToHex(pos);
	startingPos = pos;
}

void CMissile::update(float dT) {
	if (collided) {
		spawnExplosion();
		//world.destroySprite(*this);
		game.deleteEntity(*this);
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
	glm::vec3 moveVec = dirVec * missileMoveSpeed * dT;
	worldPos += moveVec;
	leadingPoint += moveVec;

	collisionCheck(moveVec);

	buildWorldMatrix();
}

/** Check for a collision along the line segment from the leading point's last known
	position to where it is now. */
bool CMissile::collisionCheck(glm::vec3& moveVec) 
{	
	//first, collect any new unique hexes that we've intersected
	CHex leadingPointHex = worldSpaceToHex(leadingPoint);
	if (leadingPointHex != lastLeadingPointHex) { //we've moved at least one hex on
		intersectedHexes = getIntersectedHexes(leadingPointLastHex, leadingPoint);
	}

	//Check if we've collided with a robot in one of those hexes
	for (auto& hex: intersectedHexes) {
		//CEntity* entity = game.map->getEntityAt2(hex.first);
		CEntities entities = game.map->getEntitiesAt(hex.first);
		for (auto& entity : entities) {
			if (entity && entity != owner && entity->live) {
				if (entity->collider) {
					auto [hit, intersection] = entity->collider->segCollisionCheck(startingPos, leadingPoint);
					if (hit) {
						collisionPt = entity->worldPos;
						collidee = entity;
						entity->receiveDamage(*owner, 5);
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
			if (game.map->getHexArray()->getHexCube(hex.first).content == solidHex) {
				collided = true;
				worldPos = hex.second - (moveVec * distToPoint);
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
	CExplosion* splode = (CExplosion *) spawn::explosion("explosion", collisionPt, 1).get();
	if (collidee)
		splode->setCollidee(collidee);
}


