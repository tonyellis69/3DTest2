#include "actor.h"

#include <cmath>

#include "IHexWorld.h" //temp bug chase

#include "tigConst.h"

const float rad360 = M_PI * 2;

/** Do the necessary one-off prep work for the given action. */
void CHexActor::startAction() {
	action = getChosenAction();

	if (action == tig::actChasePlayer) {
		CGetPlayerPos getPlayerPos;
		send(getPlayerPos);

		CGetTravelPath pathRequest(hexPosition, getPlayerPos.position);
		send(pathRequest);
		travelPath2 = pathRequest.travelPath;

	}

}

/** Update the current action of the actor by the time passed. */
bool CHexActor::update2(float dT) {
	this->dT = dT;
	switch (action) {
	case tig::actChasePlayer:
		if (navigatePath(dT)) {
			action = tig::actNone;
			return resolved;
		}
		else return unresolved;


	case tig::actNone: 
		return resolved;



	}

	return resolved;
}

/** Travel down the current travelPath. */
bool CHexActor::navigatePath(float dT) {
	CHex& currentDest = travelPath2[0];

	if (isFacing(currentDest)) {
		TActorBlock block = isBlocked(currentDest);
		if (block == currentBlocked)
			return false;
		if (block == permBlocked) {
			travelPath2.clear();
			return true;
		}

		bool reachedHex = moveTo(currentDest);
		if (reachedHex) {
			updateMapPos(currentDest);
			travelPath2.erase(travelPath2.begin());
			if (travelPath2.size() == 0)
				return true;
		}
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
		buildWorldMatrix();
		return true;
	}

	worldPos += frameTravel;
	buildWorldMatrix();
	return false;
}

void CHexActor::updateMapPos(CHex& newHex) {
	setPosition(newHex);
	CMoveEntity msg(this, newHex);
	send(msg);
}

/** Returns whether hex is blocked by an actor, and if they've stopped acting. */
TActorBlock CHexActor::isBlocked(CHex& hex) {
	CActorBlock msg(hex);
	send(msg);
	if (msg.blockingActor == NULL)
		return notBlocked;

	if (msg.blockingActor->action == tig::actNone)
		return permBlocked;

	return currentBlocked;
}




