#include "robot.h"

#include "utils/log.h"

CRobot::CRobot() {
	movePoints2 = 4;
}

/** Choose an action for the upcoming turn.  */
void CRobot::chooseTurnAction() {
	callTig(tig::onChooseTurnAction);
	action = getChosenAction();

	switch (action) {
	case tig::actChasePlayer: {
		CAddActor msg(this, actionSimul);
		send(msg);
		return;
	}
	case tig::actAttackPlayer: {
		CAddActor msg(this, actionSerial);
		send(msg);
		return;
	}
	case tig::actDither:
		setAction(tig::actNone);
		//TO DO: do something more interesting
		return;
	case tig::actShootPlayer: {
		CAddActor msg(this, actionSerial);
		send(msg);
		return;
	}
	}
}


void CRobot::receiveDamage(CGameHexObj& attacker, int damage) {
	callTig(tig::onReceiveDamage, attacker, damage);
}


void CRobot::draw() {
	CHexObject::draw();
}

/** Respond to the player's left-click/primary action on this robot. */
void CRobot::leftClick() {
	CGetPlayerPos msg;
	send(msg);

	if (isNeighbour(msg.position)) {
		CSetPlayerAction actMsg(tig::actPlayerMeleeAttack, hexPosition);
		send(actMsg);
		return;
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
		CPopupText statusPop(statusPopup, status);
		send(statusPop);
	}
}






