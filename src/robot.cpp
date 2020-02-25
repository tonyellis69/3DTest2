#include "robot.h"

#include "utils/log.h"


CRobot::CRobot() {

}

/** Choose an action for the upcoming turn. */
void CRobot::chooseTurnAction() {
	callTig(tig::onChooseTurnAction);
}

/** Initiate this turn's action. */
void CRobot::beginTurnAction() {
	CHex playerPos = hexWorld->getPlayerPosition();
	travelPath.clear();

	int action = getCurrentAction();
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
		callTig(tig::onHitPlayer);
	}
}



/** Set up any necessary cosmetic action required before the turn ends, such as turning to face
	an adjacent player. */
bool CRobot::postAction() {
	int action = getCurrentAction();
	if (action ==  tig::actTrackPlayer) {
		tigMemberInt(tig::action) = tig::actNone;
		moving = false;
		return false;
	}
	
	bool wasResolvingSerial = isResolvingSerialAction();

	tigMemberInt(tig::action) = tig::actNone;
	CHex playerDestination = hexWorld->getPlayerDestinationCB();
	if ( isNeighbour(*hexWorld->getPlayerObj()) ) {
		if (initTurnToAdjacent(playerDestination)) {
			tigMemberInt(tig::action) = tig::actTrackPlayer;
			return wasResolvingSerial ? false : true; //Sign off any previous serial action as resolved
		}
	}
	return false;
}

bool CRobot::update(float dT) {
	int action = getCurrentAction();
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
	callTig(tig::onReceiveDamage, attacker, damage);
}


int CRobot::tigCall(int memberId) {
	if (memberId == tig::isNeighbour) {
		CTigObjptr* gameObj = getParamObj(0)->getCppObj();
		CGameHexObj* hexObj = (CGameHexObj*) gameObj;
		return isNeighbour(*hexObj);
	}

}

