#include "robot.h"

#include "utils/log.h"

CRobot::CRobot() {
	movePoints2 = 4;
	tmpHP = 1;
}

/** Choose an action for the upcoming turn.  */
void CRobot::chooseTurnAction() {
	callTig(tig::onChooseTurnAction);
	action = getChosenAction();

	CGetPlayerObj getPlayer;
	send(getPlayer);
	

	switch (action) {
	case tig::actChasePlayer: {
		CAddActor msg(this, actionSimul);
		send(msg);
		return;
	}
	case tig::actAttackPlayer: {
		actionTarget = getPlayer.playerObj;
		CAddActor msg(this, actionSerial);
		send(msg);
		return;
	}
	case tig::actDither:
		setAction(tig::actNone);
		//TO DO: do something more interesting
		return;
	case tig::actShootPlayer: {
		actionTarget = getPlayer.playerObj;
		CAddActor msg(this, actionSerial);
		send(msg);
		return;
	}
	}
}

void CRobot::draw() {
	CHexObject::draw();
}

/** Respond to the player's left-click/primary action on this robot. */
void CRobot::leftClick() {
	CGetPlayerPos msg;
	send(msg);

	if (isNeighbour(msg.position)) {
		CSetPlayerAction actMsg(tig::actPlayerMeleeAttack, this);
		send(actMsg);
		return;
	}

	//assume shooting for now
	CSetPlayerAction actMsg(tig::actPlayerShoot, this);
	send(actMsg);
}

/** Respond to left-click/primary action in power mode. */
void CRobot::leftClickPowerMode() {
	CReserveNextPower msg(this);
	send(msg);
}

/** Deliver melee damage to our current target. */
void CRobot::hitTarget() {
	if (actionTarget == NULL)
		return;

	//TO DO: determine weapon by what is actually equipped, not action
	if (action == tig::actAttackPlayer) {
		actionTarget->receiveDamage(*this, 1);
	}
	else if (action == tig::actShootPlayer) {
		actionTarget->receiveDamage(*this, 2);
	}


}


int CRobot::tigCall(int memberId) {
	if (memberId == tig::isNeighbour) {
		CTigObjptr* gameObj = getParamObj(0)->getCppObj();
		CGameHexObj* hexObj = (CGameHexObj*) gameObj;
		return isNeighbour(*hexObj);
	}
	return 0;
}

void CRobot::onNotify(COnNewHex& msg) {
	if (msg.newHex == hexPosition) {
		std::string status = callTigStr(tig::getStatus);
		CSendText statusPop(statusPopup, status);
		send(statusPop);
	}
}

/** What to do when someone dies. */ 
void CRobot::deathRoutine() {
	std::string deathLog = "\n" + getName() + " destroyed!";

	CSendText msg(combatLog, deathLog);
	send(msg);

	CKill killMsg(this);
	send(killMsg);
}






