#include "gamehextObj.h"

#include <cmath>

#include "tigConst.h"

#include "IHexWorld.h"

CGameHexObj::CGameHexObj() {
	isRobot = false;
	lungeSpeed = 3.0f;
	meleeDamage = 1;
	blocks = true;
	deleteMe = false;
}

void CGameHexObj::setMap(IGameHexArray* map) {
	this->map = map;
}


void CGameHexObj::setHexWorld(IHexWorld* obj) {
	hexWorld = obj;
}


bool CGameHexObj::isResolvingSerialAction() {
	int tmpAction = tigMemberInt("action");
	return tmpAction & tig::actSerial;

}



void CGameHexObj::calcTravelPath(CHex& target) {
	//ordinarily, just find the path from where we are now.
	//but if we've moving, makes sense that the new path will start
	//where we end up
	if (currentAction.actionId == tig::actPlayerMove)
		travelPath = hexWorld->calcPath(destination, target);
	else
		travelPath = hexWorld->calcPath(hexPosition, target);
}

/**	Initialise this object to start moving to the next hex on its current travel path when it gets
	updated. This may include rotating to face that hex. */
bool CGameHexObj::beginMove() {
	if (travelPath.empty() || hexWorld->isBlockerMovingTo(travelPath[0]))
		return false;
	initMoveToAdjacent(travelPath[0]);
	initTurnToAdjacent(travelPath[0]);
	return true;
}

/** Deliver damage to our current target. */
void CGameHexObj::hitTarget() {
	attackTarget->receiveDamage(*this, meleeDamage);
}

/** Initialise an action of rotating to face the given target. */
void CGameHexObj::beginTurnToTarget(CHex& target) {
	destinationAngle = 0;
	destinationAngle = hexAngle(hexPosition, target);
	float shortestRotation = fmod(2*M_PI + destinationAngle - rotation, 2*M_PI);
	if (shortestRotation > M_PI)
		shortestRotation = -(2 * M_PI - shortestRotation);

	//moving = true;
	turning = true;
	rotationalVelocity = (shortestRotation > 0) - (shortestRotation < 0);
}

bool CGameHexObj::isNeighbour(CGameHexObj& obj) {
	return ::isNeighbour(hexPosition,obj.hexPosition);
}

int CGameHexObj::getChosenAction() {
	return tigMemberInt(tig::action);
}

std::string CGameHexObj::getName() {
	return tigMemberString(tig::name);
}

/** Called when the mouse pointer enters this entity's hex. */
void CGameHexObj::onMouseOver() {
	callTigStr(tig::onMouseOver);

}


void CGameHexObj::receiveDamage(CGameHexObj& attacker, int damage) {
	;// callTig("onReceiveDamage", attacker, damage);
}

/** Advance the lunge animation. */
bool CGameHexObj::updateLunge(float dT) {
	float lungeDistance = animCycle * 2.0f - 1.0f;

	lungeDistance = 1.0f - pow(abs(lungeDistance), 0.6f);
	lungeDistance *= hexWidth;
	glm::vec3 lungeVec = moveVector * lungeDistance;

	worldPos = cubeToWorldSpace(hexPosition) + lungeVec;
	buildWorldMatrix();

	animCycle += dT * lungeSpeed;
	if (animCycle > 1.0f) {
		setPosition(hexPosition); //ensures we don't drift.
		hitTarget();
		return false;
	}
	else {
		return true;
	}
}
