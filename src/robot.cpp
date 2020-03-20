#include "robot.h"

#include "utils/log.h"

#include "ihexworld.h"
#include "bolt.h"
#include "gameHexArray.h"

CRobot::CRobot() {

}

/** Choose an action for the upcoming turn. */
void CRobot::chooseTurnAction() {
	callTig(tig::onChooseTurnAction);
	currentAction = tig::actNone;
	int chosenAction = getChosenAction();
	if (chosenAction == tig::actChasePlayer) {
		actions.push(tig::actTurnToTarget);
		actions.push(tig::actChasePlayer);
		return;
	}
	actions.push(getChosenAction());

}

/** Initiate this turn's action. */
void CRobot::beginTurnAction() {
	return;
	//TO DO: scrap function!



	CHex playerPos = hexWorld->getPlayerPosition();
	travelPath.clear();

	int action = getChosenAction();
	if (action == tig::actChasePlayer) {
		beginChasePlayer();
	} 
	else if (action == tig::actAttackPlayer) {
		meleeAttackPlayer();
	}
	else if (action == tig::actShootPlayer) {
		shootPlayer();
	}
}

bool CRobot::update(float dT) {
	if (currentAction == tig::actNone) {
		if (actions.empty())
			return false; //TO DO: temp, removable if I implement a simulActions list
		currentAction = actions.top();
		actions.pop();

		//setup - put in its own function
		if (currentAction == tig::actChasePlayer) {
			beginChasePlayer();
		}
		else if (currentAction == tig::actAttackPlayer) {
			meleeAttackPlayer();
		}
		else if (currentAction == tig::actShootPlayer) {
			shootPlayer();
		}
		else if (currentAction == tig::actTurnToTarget) {
			beginTurnToTarget(hexWorld->getPlayerDestination());

		}

	}

	bool resolving = false;

	if (currentAction == tig::actChasePlayer || currentAction == tig::actTurnToTarget)
		resolving = updateMove(dT);

	if (currentAction == tig::actAttackPlayer) {
		resolving = updateLunge(dT);
	}

	if (currentAction == tig::actTrackPlayer) {
		resolving = updateRotation(dT);
	}

	if (!resolving) {
		currentAction = tig::actNone;
		if (actions.empty())
			return false;
	}

	return true;

	/*
	int action = getChosenAction();
	if (action == tig::actNone || action == tig::actDead )
		return false;
	
	bool resolving = false;

	if (action == tig::actChasePlayer)
		resolving = updateMove(dT);
	
	if (action == tig::actAttackPlayer) {
		resolving = updateLunge(dT);
	}

	if (action ==  tig::actTrackPlayer) {
		resolving = updateRotation(dT);
	}


	if (!resolving) {
		return afterResolving();
	}
	*/
	return resolving;
}


void CRobot::receiveDamage(CGameHexObj& attacker, int damage) {
	callTig(tig::onReceiveDamage, attacker, damage);
}


bool CRobot::onLeftClick() {
	CGameHexObj* playerObj = hexWorld->getPlayerObj();
	if (isNeighbour(*playerObj)) {
		playerObj->beginLunge(*this);
		return true;
	}

	return false;
}

void CRobot::beginChasePlayer() {
	calcTravelPath(hexWorld->getPlayerPosition());
	///////////////////
	if (!beginMove()) {
		;// afterResolving();
	}
}

void CRobot::meleeAttackPlayer() {
	attackTarget = hexWorld->getPlayerObj();
	destinationDirection = neighbourDirection(hexPosition, attackTarget->hexPosition);
	animCycle = 0;
	moveVector = directionToVec(destinationDirection);
	callTig(tig::onHitPlayer);
}

void CRobot::shootPlayer() {
	CHex endHex = map->findLineEnd(hexPosition, hexWorld->getPlayerObj()->hexPosition);
	CBolt* boltTmp = (CBolt*)hexWorld->createBolt();
	boltTmp->setPosition(hexPosition);
	boltTmp->fireAt(endHex);
	tigMemberInt(tig::action) = tig::actNone;
}


/** Set up any necessary cosmetic action required before the turn ends, such as turning to face
	an adjacent player. */
bool CRobot::afterResolving() {
	if (getChosenAction() == tig::actTrackPlayer) {
		tigMemberInt(tig::action) = tig::actNone;
		moving = false;
		return false;
	}

	bool wasResolvingSerial = isResolvingSerialAction();
	tigMemberInt(tig::action) = tig::actNone;
	CHex playerDestination = hexWorld->getPlayerDestination();
	if (isNeighbour(*hexWorld->getPlayerObj())) {
		//if (initTurnToAdjacent(playerDestination)) {
		////	tigMemberInt(tig::action) = tig::actTrackPlayer;
			return wasResolvingSerial ? false : true; //Sign off any previous serial action as resolved
		//}
	}
	return false;
}

int CRobot::tigCall(int memberId) {
	if (memberId == tig::isNeighbour) {
		CTigObjptr* gameObj = getParamObj(0)->getCppObj();
		CGameHexObj* hexObj = (CGameHexObj*) gameObj;
		return isNeighbour(*hexObj);
	}

}

