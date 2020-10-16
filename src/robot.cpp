#include "robot.h"


#include "utils/log.h"

#include "gameState.h"


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
	this->dT = dT;
	if (canSeePlayer && world.getTurnPhase() == playerPhase) {
		trackPoint(trackingTarget->worldPos);
	}

	updateViewField();
}

bool CRobot::update(float dT) {	
	bool result = CHexActor::update(dT);

	return result;
}

void CRobot::update2(float dT) {
	this->dT = dT;

	if (state == robotSleep)
		return;

	if (state == robotChase && !travelling) {
		if (world.player->hexPosition != chaseHex) {
			chaseHex = world.player->hexPosition;
			travelPath = world.map->aStarPath(hexPosition, chaseHex);
			liveLog << "\nChaseHex at " << chaseHex.x << " " << chaseHex.y << " " << chaseHex.z;
		}

		if (::isNeighbour(hexPosition, world.player->hexPosition)) {
			state = robotSleep;
			return;
		}

	}


	//if (travelPath.empty()) {
	//	chooseRandomDestination();
	//}

	travelling = true;
	moveDest = travelPath.front();
	moveReal();
	if (hexPosition == moveDest) {
		travelPath.erase(travelPath.begin());
		if (!travelPath.empty()) {
			moveDest = travelPath.front();
		}
		
	}

}

/** TO DO probably temporary! */
void CRobot::chooseRandomDestination() {
	CHex goalHex = world.map->findRandomHex(true);
	travelPath = world.map->aStarPath(hexPosition, goalHex);
}

void CRobot::draw() {
	//if (!visibleToPlayer)
	//	return;
	for (auto hex : viewField.visibleHexes)
		hexRendr->highlightHex(hex);

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

/** Called to update whether this robot is in the
player's fov. */
void CRobot::playerSight(bool inView) {
	visibleToPlayer = inView;
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

void CRobot::onNotify(COnCursorNewHex& msg) {
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
	/*if (trackingTarget == msg.actor) {
		if (canSee(msg.actor)) {
			lastSeen = msg.newHex;
			canSeeTrackee = true;
		}
		else {
			canSeeTrackee = false;
		}

	}*/
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
		CCalcVisionField msg(hexPosition, viewField.arcHexes, true);
		send(msg);
		viewField.visibleHexes = msg.visibleHexes;

		checkForPlayer();
	}
}

/** Performed after the player moves to a new hex or we recalculate the viewfield, so
	we can respond if the player is in view. */
void CRobot::checkForPlayer() {
	CGetPlayerObj getPlayer;
	send(getPlayer);

	if (canSee(getPlayer.playerObj)) {
		lastSeen = getPlayer.playerObj->hexPosition;
		if (!canSeePlayer) {
			CSendText msg(combatLog, "\n\nPlayer spotted! Tracking on");
			send(msg);

			canSeePlayer = true;
			CPlayerSeen seenMsg(this);
			send(seenMsg);

			earlyExit = true;
			earlyExitAction = tig::actTurnToTarget;
			earlyExitTarget = getPlayer.playerObj;


		//	setGoalAttack(getPlayer.playerObj);
			trackingTarget = getPlayer.playerObj;


			lineModel.setColourR(glm::vec4(1, 0, 0, 1));

		}
		else {
			//CSendText msg(combatLog, "\n\nPlayer seen again!");
			//send(msg);
		}
	}
	else {
		if (canSeePlayer) {
			CSendText msg(combatLog, "\n\nPlayer lost!!!! (new)");
			send(msg);
		}
		canSeePlayer = false;
	}
}

/** Move realtime toward the destination hex, unless we reach it. */
void CRobot::moveReal() {
	glm::vec3 dest = cubeToWorldSpace(moveDest);
	glm::vec3 travel = dest - worldPos;

	glm::vec3 moveVec = glm::normalize(travel) * robotMoveSpeed * dT;

	float remainingDist = glm::length(travel);

	if (glm::length(moveVec) > remainingDist) {
		worldPos = dest;
		setPosition(moveDest);
		travelling = false;
		return;
	}

	worldPos += moveVec;
	buildWorldMatrix();
}



