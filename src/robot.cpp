#include "robot.h"

#include "utils/log.h"

#include "ihexworld.h"
#include "bolt.h"
#include "gameHexArray.h"

TPlayerDefence& operator++(TPlayerDefence& def, int) {
	//return def = (def == defFeint) ? defNone : static_cast<TPlayerDefence>(static_cast<int>(def) + 1);
	return def = (def == defFeint) ? defNone : TPlayerDefence(def + 1);
}

CRobot::CRobot() {
	playerDefence = defNone;
}

/** Choose an action for the upcoming turn. */
void CRobot::chooseTurnAction() {
	currentAction = { tig::actNone, this };
	callTig(tig::onChooseTurnAction);
	int chosenAction = getChosenAction();

	switch (chosenAction) {
	case tig::actChasePlayer:
		actions.push({ tig::actTurnToTargetDest, NULL });
		actions.push({ chosenAction,NULL });
		return;
	case tig::actAttackPlayer:
		actions.push({ chosenAction, this });
		actions.push({ tig::actTurnToTarget, this });
		return;
	case tig::actDither:
		actions.push({ tig::actTurnToTargetDest, NULL });
		return;
	case tig::actShootPlayer:
		actions.push({ tig::actShootPlayer, hexWorld->getPlayerObj() });
		actions.push({ tig::actTurnToTarget, hexWorld->getPlayerObj() });
		return;
	default:
		actions.push({ chosenAction, NULL });
	}
}


bool CRobot::update(float dT) {
	if (currentAction.actionId == tig::actNone) {
		if (actions.empty())
			return false; //TO DO: temp, removable if I implement a simulActions list
		currentAction = actions.top();
		actions.pop();
		initialiseCurrentAction();
	}

	bool resolving = false;

	if (currentAction.actionId == tig::actTurnToTarget || currentAction.actionId == tig::actTurnToTargetDest)
		resolving = updateRotationOnly(dT);

	if (currentAction.actionId == tig::actChasePlayer )
		resolving = updateMove(dT);

	if (currentAction.actionId == tig::actAttackPlayer) {
		resolving = updateLunge(dT);
	}

	if (!resolving) {
		currentAction.actionId = tig::actNone;
		if (actions.empty())
			return false;
	}
	return true;
}


void CRobot::receiveDamage(CGameHexObj& attacker, int damage) {
	callTig(tig::onReceiveDamage, attacker, damage);
}

void CRobot::initialiseCurrentAction() {
	switch (currentAction.actionId) {
		case tig::actChasePlayer : beginChasePlayer(); break;
		case tig::actAttackPlayer : meleeAttackPlayer(); break;
		case tig::actShootPlayer : shootPlayer(); break;
		case tig::actTurnToTarget : beginTurnToTarget(hexWorld->getPlayerPosition()); break;
		//case tig::actTurnToTargetDest : beginTurnToTarget(hexWorld->getPlayerDestination()); break;
		case tig::actTurnToTargetDest: beginTurnToTarget(hexWorld->getPlayerPosition()); break;
	}
}


bool CRobot::onLeftClick() {
	return false;
}

void CRobot::onMouseWheel(float delta) {
	if (hexWorld->isStrategyMode()) {
		playerDefence++;
		spawnDefencePanel();

	}
}

bool CRobot::beginMove() {
	return CGameHexObj::beginMove();
}

void CRobot::beginChasePlayer() {
	calcTravelPath(hexWorld->getPlayerPosition());	
	movePoints = 4;
	beginMove();
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


int CRobot::tigCall(int memberId) {
	if (memberId == tig::isNeighbour) {
		CTigObjptr* gameObj = getParamObj(0)->getCppObj();
		CGameHexObj* hexObj = (CGameHexObj*) gameObj;
		return isNeighbour(*hexObj);
	}

}

/** Create a panel for the currently selected player defence against this robot.
	This will delete any current panel. */
void CRobot::spawnDefencePanel() {
	defencePanel = new CDefencePanel();

	defencePanel->addText("New defence panel " + std::to_string(defencePanel->uniqueID));

	hexWorld->addWindow(defencePanel);

	defencePanel->resizeToFit();
	defencePanel->positionAtMousePointer();
}

