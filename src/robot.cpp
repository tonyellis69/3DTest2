#include "robot.h"

#include "utils/log.h"

#include "gameState.h"
#include "missile.h"

#include <glm/gtx/vector_angle.hpp>

#include "sound/sound.h"

#include "utils/random.h"

const float rad360 = M_PI * 2;


CRobot::CRobot() {
	viewField.setField(5, 60);
	normalColour = glm::vec4(0.3, 1, 0.3, 1); //temp!

	entityType = entRobot;
}

//void CRobot::frameUpdate(float dT) {
//	this->dT = dT;
//	if (canSeePlayer && world.getTurnPhase() == playerPhase) {
//		trackPoint(trackingTarget->worldPos);
//	}
//
//	updateViewField();
//}


void CRobot::update(float dT) {
	this->dT = dT;


	if (meleeHitCooldown > 0)
		meleeHitCooldown -= dT;

	if (missileCooldown > 0)
		missileCooldown -= dT;

	if (tranistioningToHex) {
		float angleToDest = orientationTo(cubeToWorldSpace(moveDest));
		if (abs(angleToDest) > 0.01f && state != robotEvasiveShoot) { //not lined up, so rotate
			rotateAlong(angleToDest);
			//return;
		}


		moveReal();
		//return;
	}

	if (state == robotSleep)
		return;

	if (state == robotLightSleep) {
		if (cubeDistance(hexPosition, world.player->hexPosition) <= 4) {
			setState(robotHunt);
		}
		else
			return;
		
	}



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
		world.map->movingTo(this, hexPosition, moveDest);
		tranistioningToHex = true;
	}

	if (state == robotWander) {
		moveDest = getNextTravelHex(destination);
		if (moveDest == CHex(-1)) {
			world.map->setHighlight(destination, 0.0f);
			setState(robotSleep);
			return;
		}
		world.map->movingTo(this, hexPosition, moveDest);
		tranistioningToHex = true;
	}

	if (state == robotMelee) {
		melee();
	}

	if (state == robotShoot) {

		float angleToTarget = orientationTo(targetEntity->worldPos);
		if (abs(angleToTarget) > 0.01f) { //not lined up, so rotate
			rotateAlong(angleToTarget);
			return;
		}

		if (missileCooldown <= 0) {
			if (hasLineOfSight(targetEntity)) {
				fireMissile(targetEntity);
				missileCooldown = 3;
			}
			else
				setState(robotHunt);
		}
	}

	if (state == robotEvasiveShoot) {
		//check we're not already movingToHex
		// if we are, check if we've arrived
		//set up move if not moving
		//do lined-up check as above
		//fire if lined up as above
		//float angleToTarget = orientationTo(targetEntity->worldPos);
		glm::vec3 targetVec = glm::normalize(targetEntity->worldPos - worldPos);
		float targetAngle = glm::orientedAngle(targetVec, glm::vec3(1, 0, 0), glm::vec3(0, 0, -1));

		if (!tranistioningToHex) { //try to set up a move
			float perp1 = targetAngle - M_PI_2;
			float perp2 = targetAngle + M_PI_2;
			CHex newHex1 = getNeighbour(hexPosition, angleToDir(perp1));
			CHex newHex2 = getNeighbour(hexPosition, angleToDir(perp2));

			int rand = rnd::dice(2);
			if (rand == 1 && world.map->isAvailable(newHex1)) {
				moveDest = newHex1;
				world.map->movingTo(this, hexPosition, moveDest);
				tranistioningToHex = true;
			}
			else if (world.map->isAvailable(newHex2)) {
				moveDest = newHex2;
				world.map->movingTo(this, hexPosition, moveDest);
				tranistioningToHex = true;
			}
		}

		//for now, if we're evasive shooting we should always be facing the target



		//regardless of if we're evading, try to shoot
		float angleToTarget = orientationTo(targetEntity->worldPos);

		//if (abs(angleToTarget) > 0.001f) { //not lined up, so rotate
			//rotateAlong(angleToTarget);
			//liveLog << "\n" << angleToTarget;
			rotation += angleToTarget;
			liveLog << "\ncorrecting by " << angleToTarget;
			buildWorldMatrix();
	//	}

		if (missileCooldown <= 0) {
			if (hasLineOfSight(targetEntity)) {
				fireMissile(targetEntity);
				missileCooldown = 3;
			}
			else
				setState(robotHunt);
		}

	}

	if (state == robotHunt && !tranistioningToHex) {
		//no longer transitioning, so can we see player from this new hex?
		if ( hasLineOfSight(targetEntity)) {
			//setState(robotShoot);
			setState(robotEvasiveShoot);
			return;
		}


		if (world.player->hexPosition != destination) {
			destination = world.player->hexPosition;
			//TO DO: should only update if robot can *see* target position is
			//no longer destination
		}



		moveDest = getNextTravelHex(destination);
		if (moveDest == CHex(-1)) {
			return;
		}
		world.map->movingTo(this, hexPosition, moveDest);
		tranistioningToHex = true;





	}


}


/** Switch to the given state, performing the necessary initialisation. */
void CRobot::setState(TRobotState newState) {

	switch (newState) {
	case robotWander:
		destination = world.map->findRandomHex(true);
		world.map->setHighlight(destination, 1.0f); //temp test!
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
	case robotShoot:
		targetEntity = world.player;
		missileCooldown = 1.0f;
		break;
	case robotEvasiveShoot:
		targetEntity = world.player;
		missileCooldown = 1.0f;
		break;
	case robotHunt:
		targetEntity = world.player;
		lineModel.setColourR(hostileColour);
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

/** Check if the given segment intersects this robot. */
std::tuple<bool, glm::vec3> CRobot::collisionCheck(glm::vec3& segA, glm::vec3& segB) {
	//first, try simple bounding-circle check.
	//if (lineModel.circleCollision(segA, segB))
	//	return { true, glm::vec3() };

	if (lineModel.BBcollision(segA, segB))
		return { true, glm::vec3() };

	return { false, glm::vec3()};
}


void CRobot::receiveDamage(CGameHexObj& attacker, int damage) {
	//temp!!!!!!!!!!!!!!
	liveLog << "\nHit robot!";
	world.deleteEntity(*this);
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




//CHex CRobot::getLastSeen() {
//	return lastSeen;
//}

/** Reconstruct our viewfield if it needs it. If we do, check if the player is in sight. */
void CRobot::updateViewField() {
	bool rebuilt = viewField.calculateOutline(hexPosition, rotation);
	if (rebuilt) {
		CCalcVisionField msg(hexPosition, viewField.arcHexes, true);
		send(msg);
		viewField.visibleHexes = msg.visibleHexes;

		//checkForPlayer(); TO DO: needs recreating
	}
}

/** Performed after the player moves to a new hex or we recalculate the viewfield, so
	we can respond if the player is in view. */
//void CRobot::checkForPlayer() {
//	CGetPlayerObj getPlayer;
//	send(getPlayer);
//
//	if (canSee(getPlayer.playerObj)) {
//		lastSeen = getPlayer.playerObj->hexPosition;
//		if (!canSeePlayer) {
//			CSendText msg(combatLog, "\n\nPlayer spotted! Tracking on");
//			send(msg);
//
//			canSeePlayer = true;
//			CPlayerSeen seenMsg(this);
//			send(seenMsg);
//
//			earlyExit = true;
//			earlyExitAction = tig::actTurnToTarget;
//			earlyExitTarget = getPlayer.playerObj;
//
//
//		//	setGoalAttack(getPlayer.playerObj);
//			trackingTarget = getPlayer.playerObj;
//
//
//			lineModel.setColourR(glm::vec4(1, 0, 0, 1));
//
//		}
//		else {
//			//CSendText msg(combatLog, "\n\nPlayer seen again!");
//			//send(msg);
//		}
//	}
//	else {
//		if (canSeePlayer) {
//			CSendText msg(combatLog, "\n\nPlayer lost!!!! (new)");
//			send(msg);
//		}
//		canSeePlayer = false;
//	}
//}

/** Move realtime toward the destination hex, unless we reach it. */
void CRobot::moveReal() {
	glm::vec3 dest = cubeToWorldSpace(moveDest);
	glm::vec3 travel = dest - worldPos;

	glm::vec3 moveVec = glm::normalize(travel) * robotMoveSpeed * dT;

	float remainingDist = glm::length(travel);

	if (glm::length(moveVec) > remainingDist) {
		worldPos = dest;
		world.map->movedTo(this, hexPosition, moveDest);
		setPosition(moveDest);
		tranistioningToHex = false;
		return;
	}

	worldPos += moveVec;
	buildWorldMatrix();
}

/** Perform as much of the given rotation as we can in this tick. */
void CRobot::rotateAlong(const float& angle) {
	float dRotate = float((angle > 0) - (angle < 0)) * dT * robotRotateSpeed;

	if (abs(dRotate) > abs(angle)) {
		liveLog << "\nOvershot";
		dRotate = angle;
	}
	liveLog << "\nUndershot";

	rotation += dRotate;
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

/** Return true if we can draw a line to the target without hitting anything. */
bool CRobot::hasLineOfSight(CGameHexObj* target) {
	THexDir exitDir; glm::vec3 intersection;
	CHex startHex = hexPosition;

	glm::vec3 targetPos = targetEntity->worldPos;
	CHex targetHex = worldSpaceToHex(targetPos);
	while (startHex != targetHex) {
		//A += glm::vec3(1e-4, 2e-4, 0);
		//B -= glm::vec3(1e-4, 2e-4, 0);
		std::tie(exitDir, intersection) = world.map->findSegmentExit(worldPos , targetPos, startHex);

		if (exitDir == hexNone) {
			liveLog << "\nLoS fail!";
			std::tie(exitDir, intersection) = world.map->findSegmentExit(worldPos, targetPos, startHex);
		}
			//continue; //!!!!!!!!!!!!!!!!!!!!!!!!!still get infinte loop here sometimes
		//leave this in until you solve it!
		//NB may have solved it via offset in findSegmentExit 10/02/2021

		CHex entryHex = getNeighbour(startHex, exitDir);
		
		if (world.map->getHexCube(entryHex).content != emptyHex)
			return false;

		startHex = entryHex;
	}

	return true;
}

void CRobot::fireMissile(CGameHexObj* target) {
	auto missile = std::make_shared<CMissile>();

	glm::vec3 targetVec = target->worldPos - worldPos;
	float targetAngle = glm::orientedAngle(glm::normalize(targetVec), glm::vec3(1, 0, 0), glm::vec3(0, 0, -1));

	missile->setPosition(worldPos, targetAngle);
	missile->setOwner(this);
	world.addSprite(missile);

	snd::play("shoot");
}



