#include "robot.h"

#include "utils/log.h"


CRobot::CRobot() {
	action = actNone;
}

void CRobot::chooseTurnAction() {
	//is player in adjacent hex? Attack!
	CHex playerPos = hexWorld->getPlayerPositionCB();
	THexDir playerAdjacent = neighbourDirection(hexPosition, playerPos);

	if (playerAdjacent != hexNone) {
		action = actAttackPlayer;

	}
	else {
		 action = actChasePlayer;
	}

}


void CRobot::beginTurnAction() {
	CHex playerPos = hexWorld->getPlayerPositionCB();
	CHex playerDestination = hexWorld->getPlayerDestinationCB();
	travelPath.clear();

	if (action == actChasePlayer) {
		THexDir playerAdjacent = neighbourDirection(hexPosition, playerDestination);
		if (playerAdjacent == hexNone) {
			findTravelPath(hexWorld->getPlayerPositionCB());
		}
		if (!beginMove()) {
			onEndOfAction();
		}
		
	} 
	else if (action == actAttackPlayer) {
		targetDirection = neighbourDirection(hexPosition, playerPos);
		animCycle = 0;
		moveVector = directionToVec(targetDirection);
	}

}

/** Start an action where the robot will turn on the spot to face the player. */
void CRobot::beginTurnToPlayer() {
	CHex playerDestination = hexWorld->getPlayerDestinationCB();
	destinationDirection = neighbourDirection(hexPosition, playerDestination);
	float rotationDir = shortestRotation(facing, destinationDirection);
	if (rotationDir == 0.0f)
		return;
	turning = true;
	moving = true;
	rotationalVelocity = (rotationDir > 0) - (rotationDir < 0);
	destinationAngle = dirToAngle(destinationDirection);
}

/** Set up any necessary cosmetic action required before the turn ends, such as turning to face
	an adjacent player. */
void CRobot::onEndOfAction() {
	action = actNone;
	//are we adjacent to player?
	CHex playerDestination = hexWorld->getPlayerDestinationCB();
	THexDir playerAdjacent = neighbourDirection(hexPosition, playerDestination);
	if (playerAdjacent != hexNone) {
		destinationDirection = playerAdjacent;
		float rotationDir = shortestRotation(facing, destinationDirection);
		if (rotationDir == 0.0f)
			return;
		rotationalVelocity = (rotationDir > 0) - (rotationDir < 0);
		destinationAngle = dirToAngle(destinationDirection);
		action = actTrackPlayer;
		moving = true;
		turning = true;
		return;
	}


	return ;
}

bool CRobot::update(float dT) {
//	if (action == actNone)
	//	return false;

	bool resolving = CHexObject::update(dT);

	if (action == actAttackPlayer) {
		if (!attack()) {
			onEndOfAction(); 
			return false;
		}
		else {
			
			return true;
		}
	}

	if (action == actTrackPlayer) {
		if (!turning) {
			action = actNone;
			moving = false;
			return false;
		}

	}


	
	/*if (!moving) {
		//are we adjacent to player?
		//then turn to face him before we end our turn
		for (int neighbourDir = hexEast; neighbourDir <= hexNE; neighbourDir++) {
			CHex neighbour = getNeighbour(hexPosition, (THexDir)neighbourDir);
			CHexObject* entity = hexWorld->getEntityAt(neighbour);
			if (entity && !entity->isRobot) {
				if (facing != neighbourDir) {
					destinationDirection = (THexDir)neighbourDir;
					float rotationDir = shortestRotation(facing, destinationDirection);
					moving = true;
					turning = true;
					rotationalVelocity = (rotationDir > 0) - (rotationDir < 0);
					destinationAngle = dirToAngle(destinationDirection);
					return false;

				}

			}
				
		}
	}*/

	if (!resolving) {
		onEndOfAction();
	}


	return resolving;
}




