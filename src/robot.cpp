#include "robot.h"


#include "utils/log.h"


const float rad360 = M_PI * 2;


CRobot::CRobot() {
	movePoints2 = 4;

	CDiceRoll msg(3);
	send(msg);

	tmpHP = 6 + msg.result - 2;
	tmpOrigHP = tmpHP;

	viewField.setField(5, 60);
}

/** Choose an action for the upcoming turn.  */
void CRobot::chooseTurnAction() {
	callTig(tig::onChooseTurnAction); //get Tig action
	action = getChosenAction();


	chooseTurnAction2();

	return;

	CGetPlayerObj getPlayer;
	send(getPlayer);
	
	//queue up the action
	//TO DO: repetitive. Also, can this process be merged with/replace initAction?
	//feels like we're initialising twice
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
		return;
	case tig::actShootPlayer: {
		actionTarget = getPlayer.playerObj;
		CAddActor msg(this, actionSerial);
		send(msg);
		return;
	}
	case tig::actWander: {
		CAddActor msg(this, actionSimul);
		send(msg);
		return;
	}
	}
}

void CRobot::frameUpdate(float dT) {
	if (viewField.update(hexPosition, rotation)) {
		CFindVisionField msg(hexPosition, viewField.arcHexes);
		send(msg);
		viewField.visibleHexes = msg.visibleHexes;
		CGetPlayerPos playerPos;
		send(playerPos);
		checkView(playerPos.position);
	}
}

bool CRobot::update(float dT) {	
	bool result = CHexActor::update(dT);

	return result;
}

void CRobot::draw() {
	CHexObject::draw();

	for (auto hex : viewField.visibleHexes)
		hexRendr->highlightHex(hex);
}

/** Respond to the player's left-click/primary action on this robot. */
void CRobot::leftClick() {
	CGetPlayerPos msg;
	send(msg);

	//ensure a power blob is assigned to this attack
	CGetPlayerObj getPlayer;
	send(getPlayer);  

	CFindPowerUser pwrMsg(getPlayer.playerObj, true);
	send(pwrMsg);
	if (pwrMsg.power == 0) //none available apparently
		return;


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

	CDiceRoll msg(3);
	send(msg);
	int damage = 6 + msg.result - 2;

	actionTarget->receiveDamage(*this, damage);

}

int CRobot::getMissileDamage() {
	CDiceRoll msg(3);
	send(msg);
	return 6 + msg.result - 2;
}

/** Check if the given hex lies in this robot's view field. */
void CRobot::checkView(CHex& hex) {
	if (viewField.searchView(hex)) {
		CSendText msg(combatLog, "\n\nPlayer spotted!");
		send(msg);
	}
}


int CRobot::tigCall(int memberId) {
	if (memberId == tig::isNeighbour) {
		CTigObjptr* gameObj = getParamObj(0)->getCppObj();
		CGameHexObj* hexObj = (CGameHexObj*) gameObj;
		return isNeighbour(*hexObj);
	}
	else if (memberId == tig::isLineOfSight) {
		CTigObjptr* gameObj = getParamObj(0)->getCppObj();
		CGameHexObj* hexObj = (CGameHexObj*)gameObj;
		CLineOfSight msg(hexPosition, hexObj->hexPosition);
		send(msg);
		return msg.result;
	}
	return 0;
}

void CRobot::onNotify(COnNewHex& msg) {
	if (msg.newHex == hexPosition) {
		std::string status = callTigStr(tig::getStatus);
		if (tmpOrigHP > tmpHP) {
			status += "\nCondition: damaged";
		}
		CSendText statusPop(statusPopup, status);
		send(statusPop);
	}
}

void CRobot::onNotify(CPlayerNewHex& msg) {
	checkView(msg.newHex);
}

/** What to do when someone dies. */ 
void CRobot::deathRoutine() {
	std::string deathLog = "\n" + getName() + " destroyed!";

	CSendText msg(combatLog, deathLog);
	send(msg);

	CKill killMsg(this);
	send(killMsg);
}






