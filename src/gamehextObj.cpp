#include "gamehextObj.h"

#include "tigConst.h"

#include "utils/log.h"

#include "..\3Dtest\src\hexRenderer.h"


CGameHexObj::CGameHexObj() {
	lungeSpeed = 3.0f;
	meleeDamage = 1;
	mBlocks = blocksAll;
	deleteMe = false;

	static int tmpIdCounter = 1;
	tmpId = tmpIdCounter++;
}

void CGameHexObj::setLineModel(const std::string& name) {
	lineModel = hexRendr2.getLineModel(name);
	physics.boundingRadius = glm::length(lineModel.model.extents.furthestVert);
}

void CGameHexObj::draw() {
	//if (!visibleToPlayer)
	//	return;
	//!!!temporarily disabled to see map properly
	 
	
	if (hexPosition == CHex(-1))
		return;
	CHexObject::draw();
}



bool CGameHexObj::isNeighbour(CGameHexObj& obj) {
	return ::isNeighbour(hexPosition,obj.hexPosition);
}

bool CGameHexObj::isNeighbour(CHex& hex) {
	return ::isNeighbour(hexPosition, hex);
}

int CGameHexObj::getChosenAction() {
	return tigMemberInt(tig::action);
}

std::string CGameHexObj::getName() {
	return tigMemberString(tig::name);
}


/** Returns full blocking details as bit flags. */
unsigned int CGameHexObj::blocks() {
	return mBlocks;
}

/** Returns true if this entity blocks entry from the given direction. */
bool CGameHexObj::blocks(THexDir direction) {
	unsigned int dirBit = 1 << direction;
	return blocks() & dirBit;
}

/** Called to update whether this entity is in the 
player's fov. */
void CGameHexObj::playerSight(bool inView) {
	if (inView)
		visibleToPlayer = true; //will stay in view by default
}

/** Return shortest angle of rotation between the way we're facing and the given point. */
float CGameHexObj::orientationTo(glm::vec3& targetPos) {
	//find direction to target
	glm::vec3 targetDir = targetPos - worldPos;
	targetDir = glm::normalize(targetDir);
	float targetAngle = glm::acos(glm::dot(targetDir, glm::vec3(1, 0, 0)));

	//convert from [0 - pi] angle to [0 - 2pi], ie full circle
	if (targetDir.y > 0)
		targetAngle = 2 * M_PI - targetAngle;


	float PI_2 = 2 * M_PI;
	//find shortest angle between this and our direction
	float dist = fmod(PI_2 + targetAngle - rotation, PI_2);

	//put in range [-pi - pi] to give angle a direction, ie, clockwise/anti
	if (dist > M_PI)
		dist = -(PI_2 - dist);

	return dist;
}

/** Modify worldspace position by the given vector. */
void CGameHexObj::updatePos(glm::vec3& dPos) {
	worldPos += dPos;
	buildWorldMatrix();
	CHex newHexPosition = worldSpaceToHex(worldPos);
	if (newHexPosition != hexPosition) {
		hexPosition = newHexPosition;
		onMovedHex();
	}
}


void CGameHexObj::receiveDamage(CGameHexObj& attacker, int damage) {

}

/** Returns false if this was fatal. */
bool CGameHexObj::reduceHitPoints(int damage) {
	tmpHP -= damage;
	if (tmpHP <= 0) {
		deathRoutine();
		return false;
	}
	return true;
}
