#include "robot.h"

#include "utils/log.h"


CRobot::CRobot() {

}

/** Choose an action for the upcoming turn. */
void CRobot::chooseTurnAction() {
	if ( isNeighbour(hexWorld->getPlayerPosition()) ) 
		action = actAttackPlayer;
	else 
		 action = actChasePlayer;
	
}

/** Initiate this turn's action. */
void CRobot::beginTurnAction() {
	CHex playerPos = hexWorld->getPlayerPosition();
	travelPath.clear();

	if (action == actChasePlayer) {
		calcTravelPath(playerPos);
		if (!beginMove()) {
			postAction();
		}	
	} 
	else if (action == actAttackPlayer) {
		destinationDirection = neighbourDirection(hexPosition, playerPos);
		animCycle = 0;
		moveVector = directionToVec(destinationDirection);
	}
}



/** Set up any necessary cosmetic action required before the turn ends, such as turning to face
	an adjacent player. */
bool CRobot::postAction() {
	if (action == actTrackPlayer) {
		action = actNone;
		moving = false;
		return false;
	}
	
	bool wasResolvingSerial = isResolvingSerialAction();

	action = actNone;
	CHex playerDestination = hexWorld->getPlayerDestinationCB();
	if ( isNeighbour(playerDestination) ) {
		if (initTurnToAdjacent(playerDestination)) {
			action = actTrackPlayer;
			return wasResolvingSerial ? false : true; //Sign off any previous serial action as resolved
		}
	}
	return false;
}

bool CRobot::update(float dT) {
	if (action == actNone)
		return false;
	
	bool resolving = false;

	if (action == actChasePlayer)
		resolving = updateMove(dT);
	

	if (action == actAttackPlayer) {
		resolving = updateLunge(dT);////////////
		if (!resolving)
			int b = 0;
	}


	if (action == actTrackPlayer) {
		resolving = updateRotation(dT);
	}


	if (!resolving) {
		return postAction();
	}

	return resolving;
}




