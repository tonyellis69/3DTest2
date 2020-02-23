#include "robot.h"

#include "utils/log.h"


CRobot::CRobot() {
	hitPoints = 2;
}

/** Choose an action for the upcoming turn. */
void CRobot::chooseTurnAction() {

	callTig("chooseTurnAction");

	/*int action = getMemberInt("action");
	if (action == tig::actDead)
		return; //TO DO never seem to get here!!!!
	else if (isNeighbour(*hexWorld->getPlayerObj()))
		setMember("action",attackOrNot()); 
	else 
		setMember("action",tig::actChasePlayer);
	*/		
}

/** Initiate this turn's action. */
void CRobot::beginTurnAction() {
	CHex playerPos = hexWorld->getPlayerPosition();
	travelPath.clear();

	int action = getMemberInt("action");
	if (action == tig::actChasePlayer) {
		calcTravelPath(playerPos);
		if (!beginMove()) {
			postAction();
		}	
	} 
	else if (action == tig::actAttackPlayer) {
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
	int action = getMemberInt("action");
	if (action ==  tig::actTrackPlayer) {
		setMember("action", tig::actNone);
		moving = false;
		return false;
	}
	
	bool wasResolvingSerial = isResolvingSerialAction();

	setMember("action", tig::actNone);
	CHex playerDestination = hexWorld->getPlayerDestinationCB();
	if ( isNeighbour(*hexWorld->getPlayerObj()) ) {
		if (initTurnToAdjacent(playerDestination)) {
			setMember("action", tig::actTrackPlayer);
			return wasResolvingSerial ? false : true; //Sign off any previous serial action as resolved
		}
	}
	return false;
}

bool CRobot::update(float dT) {
	int action = getMemberInt("action");
	if (action == tig::actNone || action == tig::actDead )
		return false;
	
	bool resolving = false;

	if (action == tig::actChasePlayer)
		resolving = updateMove(dT);
	

	if (action == tig::actAttackPlayer) {
		resolving = updateLunge(dT);////////////
	}


	if (action ==  tig::actTrackPlayer) {
		resolving = updateRotation(dT);
	}



	if (!resolving) {
		return postAction();
	}

	return resolving;
}


void CRobot::receiveDamage(CGameHexObj& attacker, int damage) {
	int action = getMemberInt("action");
	if (action == tig::actDead) {
		liveLog << "\nYou're flogging a dead robot";
		return;
	}

	callTig("receiveDamage", attacker, damage);
	
	if (hitPoints <= 0) {
		liveLog << "\nYou trash the robot!";
		setMember("action", tig::actDead);
	}
}

int CRobot::attackOrNot() {
	int roll = hexWorld->diceRoll(2);
	if (roll == 1) {
		liveLog << "\nRobot dithers!";
		return  tig::actDither;
	}
	return tig::actAttackPlayer;
}

int CRobot::tigCall(int memberId) {
	if (memberId == getMemberId("isNeighbour")) {
		int objId = getParamInt(0);
		ITigObj* tigObj = getObject(objId);
		CTigObjptr* gameObj = tigObj->getCppObj();
		CGameHexObj* hexObj = (CGameHexObj*) gameObj;
		int result = isNeighbour(*hexObj);
		return result;
	}

}

