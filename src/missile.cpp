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
}

void CMissile::update(float dT) {
	if (collided)
		return;
	this->dT = dT;
	moveReal();
}

void CMissile::draw() {
	hexRendr2.drawLineModel(lineModel);
}


/** Move realtime in the current  direction. */
void CMissile::moveReal() {
	glm::vec3 moveVec = dirVec * missileMoveSpeed * dT;
	worldPos += moveVec;

	collisionCheck(moveVec);

	buildWorldMatrix();
}

/** Check for a collision along the line segment from the leading point's last known
	position to where it is now. */
bool CMissile::collisionCheck(glm::vec3& moveVec) {
	leadingPoint += moveVec;
	CHex leadingPointHex = worldSpaceToHex(leadingPoint);

	if (leadingPointHex != lastLeadingPointHex) { //leading point entered new hex, so check for collision
		THexDir exitDir = hexNone; glm::vec3 intersection; 
		CHex startHex = lastLeadingPointHex;

		while (startHex != leadingPointHex) {
			std::tie(exitDir, intersection) = world.map->findSegmentExit(leadingPointLastHex, leadingPoint, startHex);
			CHex entryHex = getNeighbour(startHex, exitDir);
			if (world.map->getHexCube(entryHex).content == solidHex) {
				collided = true;
				worldPos = intersection - (moveVec * distToPoint);
				return true;
			}
			startHex = entryHex;
		}
		lastLeadingPointHex = leadingPointHex;
	}

	return false;
}
