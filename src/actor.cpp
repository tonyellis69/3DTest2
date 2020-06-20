#include "actor.h"

#include "tigConst.h"

const float rad360 = M_PI * 2;

/** Do the necessary one-off prep work for the given action. */
void CHexActor::startAction(int newAction) {
	action = newAction;

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
	switch (action) {
	case tig::actAttackPlayer: return navigatePath(dT);


	}
	return false;
}

/** Travel down the current travelPath. */
bool CHexActor::navigatePath(float dT) {
	//get next hex
	//if facing next hex
	//travel to next hex



	return false;
}

bool CHexActor::isFacing(CHex& targetHex) {

	float targetDir = hexAngle(hexPosition, targetHex);
	if (rotation == targetDir)
		return true;

	glm::vec3 diffVec = cubeToWorldSpace(targetHex) - cubeToWorldSpace(hexPosition);
	glm::vec3 unitDiffVec = glm::normalize(diffVec);

	glm::vec3 frameTurnVec = unitDiffVec * dT * 10.0f; //10 may no longer be correct

	float frameTurnMag = glm::length(frameTurnVec);
	if (frameTurnMag > glm::length(diffVec)) {
		rotation = targetDir;
		buildWorldMatrix();
		return true;
	}

	//convert to radians
	//TO DO: consider storing direction as vector, saves on this:
	frameTurnVec = glm::normalize(frameTurnVec);
	rotation += glm::acos(glm::dot(frameTurnVec, glm::vec3(1, 0, 0)));
	rotation = glm::mod<float>(rotation, rad360);

	buildWorldMatrix();
	return false;



	//////////////////////////////////////////////////////
	//find total distance to turn by shortest direction
	float turnDist = rad360 + targetDir - rotation;
	turnDist = fmod(turnDist, rad360);
	if (turnDist > M_PI)
		turnDist = -(rad360 - turnDist);

	//distance to turn this frame
	float frameTurnDist = 10.0f * dT;

	//will it turn us too far? adjust
	if (frameTurnDist > abs(turnDist))
		frameTurnDist = turnDist;

	float turnDir = (turnDist > 0) - (turnDist < 0);
	rotation = rotation + turnDir * frameTurnDist;
	rotation = glm::mod<float>(rotation, rad360);

	buildWorldMatrix();
	return false;
}
