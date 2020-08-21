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
	normalColour = glm::vec4(0.3, 1, 0.3, 1); //temp!
}

void CRobot::frameUpdate(float dT) {
	updateViewField();
	
}

bool CRobot::update(float dT) {	
	bool result = CHexActor::update(dT);

	return result;
}

void CRobot::draw() {
	if (!inPlayerFov)
		return;
	//for (auto hex : viewField.visibleHexes)
	//	hexRendr->highlightHex(hex);

	CHexObject::draw();
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
		getPlayer.playerObj->setActionMelee(this);

		return;
	}

	getPlayer.playerObj->setActionShoot(this->hexPosition);

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


/** If we can see the hex this object is in, return true. */
bool CRobot::canSee(CGameHexObj* target) {
	return viewField.searchView(target->hexPosition);
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
	checkForPlayer();
}

void CRobot::onNotify(CActorMovedHex& msg) {
	if (trackingTarget == msg.actor) {
		if (canSee(msg.actor)) {
			lastSeen = msg.newHex;
		}
		else {
			lostTrackee = true;
		}

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

CHex CRobot::getLastSeen() {
	return lastSeen;
}

/** Reconstruct our viewfield if it needs it. If we do, check if the player is in sight. */
void CRobot::updateViewField() {
	bool rebuilt = viewField.calculateOutline(hexPosition, rotation);
	if (rebuilt) {
		CCalcVisionField msg(hexPosition, viewField.arcHexes);
		send(msg);
		viewField.visibleHexes = msg.visibleHexes;

		checkForPlayer();
	}
}

void CRobot::checkForPlayer() {
	CGetPlayerObj getPlayer;
	send(getPlayer);
	//return;

	if (canSee(getPlayer.playerObj) && trackingTarget == NULL) {
		CSendText msg(combatLog, "\n\nPlayer spotted!");
		send(msg);

		earlyExit = true;
		earlyExitAction = tig::actTurnToTarget;
		earlyExitTarget = getPlayer.playerObj;


	//	setGoalReact(getPlayer.playerObj);

	//	setActionTurnTo(getPlayer.playerObj->hexPosition);

		setGoalAttack(getPlayer.playerObj);
		trackingTarget = getPlayer.playerObj;
		lastSeen = getPlayer.playerObj->hexPosition;

		lineModel.setColourR(glm::vec4(1,0,0,1));

	}
}




