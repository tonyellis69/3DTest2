#include "actor.h"

#include <cmath>

#include <glm/gtx/rotate_vector.hpp>

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

	if (earlyExit && action == tig::actNone) {
		earlyExit = false;
		action = earlyExitAction;
		actionTarget = earlyExitTarget;
		targetHex = actionTarget->hexPosition;
		return unresolved;
	}
	

	return unresolved;
}







/** Travel down the current travelPath. */
bool CHexActor::navigatePath(float dT) {
	CHex destHex = travelPath[0];

	if (isFacing(destHex)) {
		if (destHexClaimed) {
			if (moveTo(destHex)) {
				travelPath.erase(travelPath.begin());
				if (travelPath.empty())
					return true;
				if (earlyExit) {
					travelPath.clear();
					return true;
				}
				destHexClaimed = false; 
			}
		}
		else if (permBlocked)
			return true; //give up
	}

	return false;
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

		onMovedHex();

		return true;
	}

	worldPos += frameTravel;
	buildWorldMatrix();
	return false;
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




/** Called when actor has arrived at a new hex.*/
void CHexActor::onMovedHex() {
	CActorMovedHex msg(hexPosition, this);
	send(msg);
}


/** Rotate in the direction of the given worldspace point. */
void CHexActor::trackPoint(glm::vec3& point) {
	glm::vec3 targetVec = glm::normalize(point - worldPos);

	float angle = glm::acos(glm::dot(targetVec, glm::vec3(1, 0, 0)));
	if (targetVec.y > 0)
		angle = 2 * M_PI - angle;

	if (rotation == angle)
		return;

	//find total distance to turn by shortest direction
	float turnDist = rad360 + angle - rotation;
	turnDist = fmod(turnDist, rad360);
	if (turnDist > M_PI)
		turnDist = -(rad360 - turnDist);

	//distance to turn this frame
	float frameTurnDist = turnSpeed * dT;

	//will it turn us too far? adjust
	if (frameTurnDist > abs(turnDist)) {
		rotation = angle;
	}
	else {
		float turnDir = (turnDist > 0) - (turnDist < 0);
		rotation = rotation + turnDir * frameTurnDist;
		rotation = glm::mod<float>(rotation, rad360);
	}
	//liveLog << "\nTurn! " << frameTurnDist;
	buildWorldMatrix();
	
}





