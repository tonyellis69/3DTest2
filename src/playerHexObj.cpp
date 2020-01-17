#include "playerHexObj.h"

bool CPlayerObject::update(float dT) {
	CHex playerOldHex = hexPosition;

	bool result = CHexObject::update(dT);

	if (action == actPlayerTurnToAttack && !turning) {
		moving = false;
		action = actPlayerAttack;

	}

	if (action == actPlayerAttack) {
		if (!attack()) {
			//onEndOfAction();
			action = actNone;
			return false;
		}
		else {

			return true;
		}
	}



	if (hexPosition != playerOldHex) {
		hexWorld->onPlayerTurnDoneCB();
	}

	return result;
}

void CPlayerObject::beginAttack(CHexObject& target) {
	action = actPlayerAttack;

	destinationDirection = neighbourDirection(hexPosition, target.hexPosition);
	targetDirection = destinationDirection;
	animCycle = 0;
	moveVector = directionToVec(targetDirection);


	float rotationDir = shortestRotation(facing, destinationDirection);
	if (rotationDir != 0.0f) {
		turning = true;
		moving = true;
		action = actPlayerTurnToAttack;
		rotationalVelocity = (rotationDir > 0) - (rotationDir < 0);
		destinationAngle = dirToAngle(destinationDirection);
	}
}
