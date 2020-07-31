#include "actor.h"

#include <cmath>

#include <glm/gtx/rotate_vector.hpp>

//#include "IHexWorld.h" //temp bug chase
#include "utils/log.h"

#include "tigConst.h"

const float rad360 = M_PI * 2;

/** Update the current action of the actor by the time passed. */
bool CHexActor::update(float dT) {
	this->dT = dT;
	switch (action) {
	case tig::actMoveTo:
		if (navigatePath(dT)) {
			action = tig::actNone;
		}
		break;

	case tig::actMelee:
		if (meleeAttack(dT)) {
			hitTarget();
			action = tig::actNone;
		}
		break;

	case tig::actShoot:
		if (shootTarget(dT)) {
			action = tig::actNone;
		}
		break;
	case tig::actTurnToTarget:
		if (turnTo(dT)) {
			action = tig::actNone;
		}
		break;

	case tig::actNone: 
		return resolved;

	}

	return unresolved;
}

void CHexActor::setActionMoveTo(CHex& hex) {
	action = tig::actMoveTo;
	targetHex = hex;
	CGetTravelPath pathRequest(hexPosition, targetHex);
	send(pathRequest);
	travelPath = pathRequest.travelPath;
	if (travelPath.size() > movePoints2)
		travelPath.resize(movePoints2);
	destHexClaimed = false;
	blockedFor = 0;
	CAddActor msg(this, actionSimul);
	send(msg);
}

void CHexActor::setActionShoot(CHex& hex) {
	action = tig::actShoot;
	this->targetHex = hex;
	CAddActor msg(this, actionSerial);
	send(msg);
}

void CHexActor::setActionMelee(CGameHexObj* target) {
	action = tig::actMelee;
	actionTarget = target;
	CAddActor msg(this, actionSerial);
	send(msg);
}

void CHexActor::setActionTurnTo(CHex& hex) {
	action = tig::actTurnToTarget;
	this->targetHex = hex;
	CAddActor msg(this, actionSimul);
	send(msg);
}


/** Travel down the current travelPath. */
bool CHexActor::navigatePath(float dT) {
	CHex destHex = travelPath[0];
	bool permBlocked = checkForBlock(destHex);

	if (isFacing(destHex)) {
		if (destHexClaimed) {
			if (moveTo(destHex)) {
				travelPath.erase(travelPath.begin());
				if (travelPath.empty())
					return true;
				destHexClaimed = false; 
			}
		}
		else if (permBlocked)
			return true; //give up
	}

	return false;
}

/** Attempt to claim this hex if we haven't already, return true if it 
	is permanently occupied for this turn. */
bool CHexActor::checkForBlock(CHex& destHex) {
	if (destHexClaimed)
		return false;
	
	CFindActorBlock msg(destHex);
	send(msg);
	if (msg.blockingActor == this)
		int b = 0;

	if (msg.blockingActor == NULL) {
		claimMapPos(destHex);
		return false;
	}

	if (msg.blockingActor->action == tig::actNone) {
		return true;
	}

	blockedFor += dT;
	if (blockedFor > 1.0f)
		return true;

	return false;;
}



/** Rotate actor towards direction of target hex, returning true if we're facing it.*/
bool CHexActor::isFacing(CHex& targetHex) {
	float targetDir = hexAngle(hexPosition, targetHex);
	if (rotation == targetDir)
		return true;

	//find total distance to turn by shortest direction
	float turnDist = rad360 + targetDir - rotation;
	turnDist = fmod(turnDist, rad360);
	if (turnDist > M_PI)
		turnDist = -(rad360 - turnDist);

	//distance to turn this frame
	float frameTurnDist = turnSpeed * dT;

	//will it turn us too far? adjust
	if (frameTurnDist > abs(turnDist)) {
		rotation = targetDir;
	}
	else {
		float turnDir = (turnDist > 0) - (turnDist < 0);
		rotation = rotation + turnDir * frameTurnDist;
		rotation = glm::mod<float>(rotation, rad360);
	}

	buildWorldMatrix();
	return false;
}

/** Move toward this hex, returning true if we've reached it. */
bool CHexActor::moveTo(CHex& hex) {
	glm::vec3 dest = cubeToWorldSpace(hex);
	glm::vec3 travel = dest - worldPos;
	float remainingDist = glm::length(travel);

	travel = glm::normalize(travel);
	glm::vec3 frameTravel = travel * dT * moveSpeed2;
	if (glm::length(frameTravel) > remainingDist) {
		worldPos = dest;
		setPosition(hex);
		buildWorldMatrix();

		CActorMovedHex msg(hex, this);
		send(msg);

		return true;
	}

	worldPos += frameTravel;
	buildWorldMatrix();
	return false;
}

/** Mark this hex as occupied so no one else tries to move into it in real-time.*/
void CHexActor::claimMapPos(CHex& newHex) {
	CMoveEntity msg(this, newHex);
	send(msg);
	destHexClaimed = true;
}

/** Continue the melee attack action. */
bool CHexActor::meleeAttack(float dT) {
	if (isFacing(targetHex)) {
		return lungeAt(targetHex);
	}
}

bool CHexActor::shootTarget(float dT) {
	if (isFacing(targetHex)) {
		CShootAt msg(hexPosition, targetHex, this, getMissileDamage());
		send(msg);
		return true;
	}

	return false;
}

bool CHexActor::turnTo(float dT) {
	if (isFacing(targetHex)) {
		return true;
	}
	return false;
}

/** Continue the lunge animation. */
bool CHexActor::lungeAt(CHex& hex) {
	float lungeDistance = animCycle * 2.0f - 1.0f;
	lungeDistance = 1.0f - pow(abs(lungeDistance), 0.6f);
	lungeDistance *= hexWidth;

	THexDir targetDir = neighbourDirection(hexPosition, hex);

	glm::vec3 lungeVec = directionToVec(targetDir) * lungeDistance;

	worldPos = cubeToWorldSpace(hexPosition) + lungeVec;
	buildWorldMatrix();

	animCycle += dT * lungeSpeed;
	if (animCycle > 1.0f) {
		setPosition(hexPosition); //ensures we don't drift.
		return true;
	}
	else {
		return false;
	}
}





