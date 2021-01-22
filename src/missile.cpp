#include "missile.h"

#include <glm/gtc/matrix_transform.hpp>

#include "gameState.h"
#include "utils/log.h"

#include "hexRenderer.h"

CMissile::CMissile() {
	lineModel = hexRendr2.getLineModel("bolt");
}

void CMissile::setPosition(glm::vec3& pos, float rotation) {
	CSprite::setPosition(pos, rotation);
	dirVec =  glm::normalize( *worldMatrix * glm::vec4(1, 0, 0,0) );
	leadingPoint = worldPos + dirVec * distToPoint;
	leadingPointLastHex = leadingPoint;
	lastLeadingPointHex = worldSpaceToHex(pos);
	startingPos = pos;
}

void CMissile::update(float dT) {
	if (collided) {
		world.destroySprite(*this);
		return;
	}
	this->dT = dT;
	moveReal();
}

void CMissile::draw() {
	hexRendr2.drawLineModel(lineModel);
}

void CMissile::setOwner(CGameHexObj* owner) {
	this->owner = owner;
}


/** Move realtime in the current  direction. */
void CMissile::moveReal() {
	glm::vec3 moveVec = dirVec * missileMoveSpeed * dT;
	worldPos += moveVec;
	leadingPoint += moveVec;

	collisionCheck(moveVec);

	buildWorldMatrix();
}

/** Check for a collision along the line segment from the leading point's last known
	position to where it is now. */
bool CMissile::collisionCheck(glm::vec3& moveVec) {
	CHex leadingPointHex = worldSpaceToHex(leadingPoint);
	THexDir exitDir = hexNone; glm::vec3 intersection;
	CHex startHex = lastLeadingPointHex;

	//first, collect unique hexes that we've intersected so far
	if (leadingPointHex != lastLeadingPointHex) { //we've moved at least one hex on
		//THexDir exitDir = hexNone; glm::vec3 intersection;
		//CHex startHex = lastLeadingPointHex;
		startHex = lastLeadingPointHex;
		exitDir = hexNone;
		while (startHex != leadingPointHex) {
			std::tie(exitDir, intersection) = world.map->findSegmentExit(leadingPointLastHex, leadingPoint, startHex);
			CHex entryHex = getNeighbour(startHex, exitDir);
			//if (entryHex == hexNone) { //errors? try reinstating this
			//	break; //hopefully catch rare case where leading point on hex border.
			//}
			intersectedHexes.insert({entryHex, intersection});
			startHex = entryHex;
		}
	}



	//Check every frame if we've collided with a robot in one of those hexes
	for (auto& hex: intersectedHexes) {
		CGameHexObj* entity = world.map->getEntityAt2(hex.first);
		if (entity && entity != owner) {
			auto [hit, intersection] = entity->collisionCheck(startingPos, leadingPoint);
			if (hit) {
				entity->receiveDamage(*owner, 10);
				collided = true;
				return true;
			}
		}
	}

	//still here? Check if we've collided with scenery - but only if we've entered a new hex.
	if (leadingPointHex != lastLeadingPointHex) {
		for (auto& hex : intersectedHexes) {
			if (world.map->getHexCube(hex.first).content == solidHex) {
				collided = true;
				worldPos = hex.second - (moveVec * distToPoint);
				return true;
			}

		}
	
		//optimisation: remove hex from list if safe to do so
		if (intersectedHexes.size() > 1) {
			auto exitedHex = intersectedHexes.find(lastLeadingPointHex);
			if (exitedHex  != intersectedHexes.end())
				intersectedHexes.erase(exitedHex);
		}
		lastLeadingPointHex = leadingPointHex;
	}


	return false;
}
