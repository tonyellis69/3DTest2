#include "robot.h"


#include "utils/log.h"

#include "gameState.h"


const float rad360 = M_PI * 2;


CRobot::CRobot() {
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


void CRobot::update2(float dT) {
	this->dT = dT;

	if (meleeHitCooldown > 0)
		meleeHitCooldown -= dT;

	if (travellingToHex) {
		moveReal();
		return;
	}

	if (state == robotSleep)
		return;

	//if the player is adjacent, hit player
	if (state == robotChase && glm::distance(worldPos, world.player->worldPos) 
			< robotMeleeRange && meleeHitCooldown <= 0) {
		setState(robotMelee);
		return;
	}


	if (state == robotChase) {
		if (world.player->hexPosition != destination) {
			destination = world.player->hexPosition;
		}

		moveDest = getNextTravelHex(destination);
		if (moveDest == CHex(-1)) {
			return;
		}
		travellingToHex = true;
	}

	if (state == robotWander) {
		moveDest = getNextTravelHex(destination);
		if (moveDest == CHex(-1)) {
			setState(robotSleep);
			return;
		}
		travellingToHex = true;
	}

	if (state == robotMelee) {
		melee();


	}


}


/** Switch to the given state, performing the necessary initialisation. */
void CRobot::setState(TRobotState newState) {

	switch (newState) {
	case robotWander:
		destination = world.map->findRandomHex(true);
		break;
	case robotChase:
		destination = world.player->hexPosition;
		break;
	case robotMelee:
		//TO DO: can set target here later
		travelPath.clear();
		destination = CHex(-1);
		meleeHitCooldown = 3.0f;
		lungeDir = glm::normalize(world.player->worldPos - worldPos);
		lungeReturning = false;
		lungeEndPos = worldPos;
		break;
	case robotSleep:
		destination = CHex(-1);
		break;
	}

	state = newState;
}

TRobotState CRobot::getState() {
	return state;
}


/** Return the next destination hex, if any, in this robot's ongoing journey. 
	If the current path is blocked, try to find a way around it. */
CHex CRobot::getNextTravelHex(CHex& destination) {
	if (travelPath.empty() || travelPath.back() != destination) {
		travelPath = world.map->aStarPath(hexPosition, destination);
		//!!!!check for blocking robots in the first hex and we can exit early
		if (travelPath.empty()) //no route to destination
			return CHex(-1);
	}

	//is previously calculated next destination hex still free ?
	if (!world.map->isFree(travelPath.front())) {
		travelPath = world.map->aStarPath(hexPosition, destination, travelPath.front());
	}

	if (travelPath.empty()) 
		return CHex(-1);

	CHex hex = travelPath.front();
	travelPath.erase(travelPath.begin());
	return hex;
}


void CRobot::draw() {
	//if (!visibleToPlayer)
	//	return;
	for (auto hex : viewField.visibleHexes)
		hexRendr->highlightHex(hex);

	CHexObject::draw();
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
		travellingToHex = false;
		return;
	}

	worldPos += moveVec;
	buildWorldMatrix();
}

/** Perform realtime melee action. Eg, lunging at target. */
void CRobot::melee() {
	worldPos += lungeDir * robotLungeSpeed * dT;
	if (glm::distance(worldPos, lungeEndPos) > robotLungeDistance) {
		if (lungeReturning) {
			setState(robotChase);
			lungeDir = glm::vec3(0);
		}
		else {
			lungeEndPos = worldPos;
			lungeDir = -lungeDir;
			lungeReturning = true;
		}
	}

	buildWorldMatrix();
}



