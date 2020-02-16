#include "robot.h"

#include "utils/log.h"


CRobot::CRobot() {
	hitPoints = 2;
}

/** Choose an action for the upcoming turn. */
void CRobot::chooseTurnAction() {
	if (action == actDead)
		return;
	else if (isNeighbour(hexWorld->getPlayerPosition()))
		action = attackOrNot(); // actAttackPlayer;
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
		attackTarget = hexWorld->getPlayerObj();
		destinationDirection = neighbourDirection(hexPosition, playerPos);
		animCycle = 0;
		moveVector = directionToVec(destinationDirection);
		liveLog << "\nA robot hits you!";
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
	if (action == actNone || action == actDead)
		return false;
	
	bool resolving = false;

	if (action == actChasePlayer)
		resolving = updateMove(dT);
	

	if (action == actAttackPlayer) {
		resolving = updateLunge(dT);////////////
	}


	if (action == actTrackPlayer) {
		resolving = updateRotation(dT);
	}


	if (!resolving) {
		return postAction();
	}

	return resolving;
}


void CRobot::receiveDamage(CHexObject& attacker, int damage) {
	if (action == actDead) {
		liveLog << "\nYou're flogging a dead robot";
		return;
	}

	int prevhitPoints = tigObj->getMemberInt("hitPoints");

	tigObj->call("receiveDamage", attacker, damage );
	

	int posthitPoints = tigObj->getMemberInt("hitPoints");

	if (hitPoints <= 0) {
		liveLog << "\nYou trash the robot!";
		action = actDead;
	}
}

int CRobot::attackOrNot() {
	int roll = hexWorld->diceRoll(2);
	if (roll == 1) {
		liveLog << "\nRobot dithers!";
		return actDither;
	}
	return actAttackPlayer;
}

