#include "gamehextObj.h"

#include <cmath>

#include "tigConst.h"

#include "IHexWorld.h"
#include "IGameHexArray.h"

CGameHexObj::CGameHexObj() {
	isRobot = false;
	lungeSpeed = 3.0f;
	meleeDamage = 1;
	mBlocks = blocksAll;
	deleteMe = false;
	currentAction = { tig::actNone,NULL };
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
	if (travelPath.empty() || movePoints == 0)
		return false;

	CGameHexObj* entity = map->getBlockingEntityAt(travelPath[0]);
	if (entity && !entity->isTigClass(tig::CDoor)) {

		if (entity->getCurrentAction() == tig::actNone) {
			currentAction.actionId = tig::actNone;
			return false; //entity not moving, give up
		}

		return true; //try again next update
	}

	movePoints--;

	map->moveEntity(this, travelPath[0]);

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

/** Returns full blocking details as bit flags. */
unsigned int CGameHexObj::blocks() {
	return mBlocks;
}

/** Returns true if this entity blocks entry from the given direction. */
bool CGameHexObj::blocks(THexDir direction) {
	unsigned int dirBit = 1 << direction;

	return blocks() & dirBit;
}

int CGameHexObj::getCurrentAction() {
	return currentAction.actionId;
}

int CGameHexObj::getNextAction() {
	if (actions.empty())
		return tig::actNone;
	return actions.top().actionId;
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
