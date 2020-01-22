#include "playerHexObj.h"

bool CPlayerObject::update(float dT) {
	CHex playerOldHex = hexPosition;

	bool resolving = false;
	resolving = updateMove(dT);

	if (action == actPlayerTurnToAttack && !turning) {
		moving = false;
		action = actPlayerAttack;
	}

	if (action == actPlayerAttack) {
		resolving = updateLunge(dT);
	}

	if (hexPosition != playerOldHex) {
		hexWorld->onPlayerTurnDoneCB();
	}

	//TO DO: can do post actions here
	if (!resolving)
		action = actNone;

	return resolving;
}

void CPlayerObject::beginAttack(CHexObject& target) {
	action = actPlayerAttack;

	if (initTurnToAdjacent(target.hexPosition))
		action = actPlayerTurnToAttack;

	animCycle = 0;
	moveVector = directionToVec(destinationDirection);
}
