#include "roboState.h"

#include <cmath>
#include <glm/gtx/vector_angle.hpp>

#include "robot.h"

#include "utils/random.h"
#include "gameState.h"

#include "utils/log.h"

const float rad360 = M_PI * 2;
const float rad80 = 1.39626;
const float rad90 = M_PI / 2;
const float rad60 = M_PI / 3;
const float rad45 = M_PI / 4;
const float rad120 = rad360 / 3;

CRoboWander::CRoboWander(CRobot* bot) : CRoboState(bot) {
	bot->chosenSpeed = bot->defaultSpeed;
	//bot->lineModel.setColourR(glm::vec4(0, 1, 0, 1));
	
	THexList ring;
	CHex randHex;
	int dist = 5;
	do {
		int giveUp = 0;
		ring = findRing(dist, bot->hexPosition);
		do {
			randHex = ring[rnd::dice(ring.size()) - 1];
			//can we los randHex?
			glm::vec3 hexWS = cubeToWorldSpace(randHex);
			if (bot->clearLineTo(hexWS)) {
				destination = hexWS;
				turnDestination = glm::orientedAngle(glm::normalize(hexWS - bot->worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
				lastDestinationDist = FLT_MAX;
				sysLog << "\nNew destination!";
				return;
			}
			giveUp++;

		} while (giveUp < 10); //temp!!!!!!!!!!!
		dist--;

	} while (dist > 0);


}

std::shared_ptr<CRoboState> CRoboWander::update(float dT) {
	if (bot->canSeePlayer())
		return std::make_shared<CCloseAndShoot>(bot, game.player);

	this->dT = dT;

	//proceed to actual movement
	float dist = glm::distance(bot->worldPos, destination);

	if (dist < 0.05f) {
		bot->stopMoving();
		bot->destinationDist = FLT_MAX;
		return std::make_shared<CGlanceAround>(bot); 
	}

	bot->headTo(destination);
	
	return nullptr;
}

glm::vec3* CRoboWander::getDestination() {
	return &destination;
}




CGlanceAround::CGlanceAround(CRobot* bot) : CRoboState(bot) {
	float leftLimit = fmod(rad360 + bot->upperBodyRotation - rad90, rad360);
	float rightLimit = fmod(rad360+ bot->upperBodyRotation + rad90, rad360);
	cumulativeRotation = 0;
	totalRotation = 0;

	float shortPause = 0.25f;
	float longPause = 0.5f;
	float currentFocus = bot->upperBodyRotation;

	bot->upperBodyLocked = false;

	glances = { {currentFocus, shortPause}, {leftLimit, shortPause}, {currentFocus,0}, {rightLimit, shortPause},
		{currentFocus, longPause} };
}

std::shared_ptr<CRoboState> CGlanceAround::update(float dT) {
	if (bot->canSeePlayer()) {
		//return std::make_shared<CCharge>(bot,game.player);
		bot->upperBodyLocked = true;
		return std::make_shared<CCloseAndShoot>(bot, game.player);
	}

	if (pause > 0) {
		pause -= dT;
		return nullptr;
	}

	if (glances.empty()) {
		bot->upperBodyLocked = true;
		return std::make_shared<CRoboWander>(bot);
	}

	TGlance dest = glances.front();

	//find shortest angle between dest angle and our direction
	float turnDist; 
	if (totalRotation == 0) {
		turnDist = fmod(rad360 + dest.angle - bot->upperBodyRotation, rad360);
		//put in range [-pi - pi] to give angle a direction, ie, clockwise/anti
		if (turnDist > M_PI)
			turnDist = -(rad360 - turnDist);

		totalRotation = abs(turnDist);
		turnDir = (std::signbit(turnDist)) ? -1.0f : 1.0f;
	}


	float frameTurn = dT * glanceSpeed * turnDir;
	cumulativeRotation += abs(frameTurn);
	if (cumulativeRotation > totalRotation) { //overshot
		bot->upperBodyRotation = dest.angle;
		totalRotation = 0;
		cumulativeRotation = 0;
		pause = dest.pause;
		glances.erase(glances.begin());



	}
	else {
		bot->upperBodyRotation += frameTurn;
		//bot->rotation = fmod(rad360 + bot->rotation, rad360); //needed??
	}

	return nullptr;
}


CCharge::CCharge(CRobot* bot, CEntity* targetEntity) : CRoboState(bot) {
	this->targetEntity = targetEntity;
}

std::shared_ptr<CRoboState> CCharge::update(float dT) {
	
	if (bot->canSeePlayer()) { //keep destination up to date
		destination = targetEntity->worldPos;
		targetInSight = true;
	}
	else
		targetInSight = false;
		//TO DO: switch to some other behaviour. Prob
		//return std::make_shared<CGoToHunting>(bot, lastSighting, targetEntity);
	
	//reached destination?
	if (glm::distance(bot->worldPos, destination) < meleeRange ) {
		bot->stopMoving();
		if (targetInSight)
			return std::make_shared<CMelee>(bot,targetEntity); 
		else {
			//bot->lineModel.setColourR(glm::vec4(0, 1, 0, 1));
			return std::make_shared<CRoboWander>(bot);
		}
	}

	//otherwise charge at target
	bot->headTo(targetEntity->worldPos);

	//ensure facing destination
	float destAngle = glm::orientedAngle(glm::normalize(destination - bot->worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
	bot->setRotation(destAngle);
	return nullptr;
}

glm::vec3* CCharge::getDestination() {
	return &destination;
}


CMelee::CMelee(CRobot* bot, CEntity* targetEntity) : CRoboState(bot) {
	this->targetEntity = targetEntity; 
	lungeVec = glm::normalize(targetEntity->worldPos - bot->worldPos);
	timer = 0;
	startPos = bot->worldPos;
}

std::shared_ptr<CRoboState> CMelee::update(float dT) {
	float targetDist = glm::distance(bot->worldPos, targetEntity->worldPos);
	if (targetDist > meleeRange && timer < 0)
		return std::make_shared<CRoboWander>(bot);

	timer += dT;
	if (timer < 0)
		return nullptr;

	if (timer < lungeEnd) {
		float step = targetDist /(lungeEnd - timer) ;
		step *= dT;
		bot->worldPos += lungeVec * step;
		return nullptr;
	}

	if (timer < returnEnd) {
		float dist = glm::distance(startPos, bot->worldPos);
		float step = dist / (returnEnd - timer) ;
		step *= dT;
		bot->worldPos += -lungeVec * step;
		return nullptr;
	}

	timer = -1.0f;
	bot->worldPos = startPos;

	return nullptr;
}


CCloseAndShoot::CCloseAndShoot(CRobot* bot, CEntity* targetEntity) : CRoboState(bot) {
	this->targetEntity = targetEntity;
	bot->startTracking(targetEntity);
	bot->chosenSpeed = bot->maxSpeed; 
}

std::shared_ptr<CRoboState> CCloseAndShoot::update(float dT) {
	missileCooldown += dT;
	

	//can we see the target?
	if (bot->clearLineTo(targetEntity)) {
		lastSighting = targetEntity->worldPos;

		if (missileCooldown > 1.0f) {
			bot->fireMissile(targetEntity);
			missileCooldown = 0;
		}

		//should we get closer?
		float targetDist = glm::distance(bot->worldPos, targetEntity->worldPos);

		if (targetDist > idealShootRange && !stoppedToShoot) {


			destination = targetEntity->worldPos;
			bot->headTo(targetEntity->worldPos);

		}
		else {
			stoppedToShoot = true;
			bot->stopMoving();
		}

		if (stoppedToShoot && targetDist > escapeRange) {
			destination = targetEntity->worldPos;
			bot->headTo(targetEntity->worldPos);

		}
	}
	else { //lost sight of target
		bot->stopTracking(); //!!!Temp! should track lastsighting instead
		return std::make_shared<CGoToHunting>(bot, lastSighting, targetEntity);
	}


	return nullptr;
}

glm::vec3* CCloseAndShoot::getDestination() {
	return &destination;
}


CGoTo::CGoTo(CRobot* bot, glm::vec3& dest) : CRoboState(bot) {
	destination = dest;
	bot->startTracking(dest);
	bot->chosenSpeed = bot->defaultSpeed;
}

std::shared_ptr<CRoboState> CGoTo::update(float dT) {
	float dist = glm::distance(bot->worldPos, destination);

	////ensure facing destination
	//float destAngle = glm::orientedAngle(glm::normalize(destination - bot->worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
	//bot->setRotation(destAngle);


	//if (dist < 0.05f) {
	if (dist < 0.2f) {	//too close causes doubling-back when avoidance is also pushing us away from destination
		bot->stopMoving();
		bot->stopTracking();
		return std::make_shared<CGlanceAround>(bot);
	}

	bot->headTo(destination);

	return nullptr;
}

glm::vec3* CGoTo::getDestination() {
	return &destination;
}



CGoToHunting::CGoToHunting(CRobot* bot, glm::vec3& dest, CEntity* quarry) : CGoTo(bot, dest) {
	targetEntity = quarry;
	bot->chosenSpeed = bot->defaultSpeed;
}

std::shared_ptr<CRoboState> CGoToHunting::update(float dT) {
	//can we see quarry? switch to melee mode - hardcode for now
	if (bot->clearLineTo(targetEntity)) {
		return std::make_shared<CCloseAndShoot>(bot, targetEntity);
	}


	CGoTo::update(dT);
	return nullptr;
}
