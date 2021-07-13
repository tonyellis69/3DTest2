#include "robot.h"

#include "utils/log.h"

#include "hexRenderer.h"

#include "gameState.h"
#include "missile.h"

#include <glm/gtx/vector_angle.hpp>

#include "sound/sound.h"

#include "utils/random.h"


const float rad360 = M_PI * 2;
const float rad60 = M_PI / 3;
const float rad45 = M_PI / 4;

CRobot::CRobot() {
	isRobot = true;
	physics.invMass = 1.0f / 80.0f; //temp!
}


void CRobot::update(float dT) {
	this->dT = dT;

	meleeHitCooldown +=dT;
	missileCooldown += dT;


	switch (state) {
	case robotWander3:
		wander3();
		break;
	case  robotLightSleep:
		if (cubeDistance(hexPosition, world.player->hexPosition) <= 4
			&& hasLineOfSight(world.player)) {
			setState(robotCharge3,world.player);
			//setState(robotCloseAndShoot, world.player);
			tracking = trackNone;
		}
		break;
	case robotCharge3:
		charge3();
		break;
	case robotMelee3:
		melee3();
		break;
	case robotCloseAndShoot:
		closeAndShoot();
		break;
	}

	if (!reachedDestination)
		approachDestination();
	
	track();

	}


/** Switch to the given state, performing the necessary initialisation. */
void CRobot::setState(TRobotState newState, CEntity* entity)
{
	switch (newState) {
	case robotWander3:
		lineModel.setColourR(glm::vec4(0, 1, 0, 1));
		//robotMoveSpeed = 3.0f;
		break;
	case robotLightSleep:
		reachedDestination = true;
		speed = 0;
		break;
	case robotCharge3:
		targetEntity = entity;
		reachedDestination = false;
		break;
	case robotMelee3:
		targetEntity = entity;
		meleeHitCooldown = 0.5f;
		meleeLungeHome = worldPos;
		lungeState = preLunge;
		break;
	case robotCloseAndShoot:
		targetEntity = entity;
		missileCooldown = 0.5f;
		stoppedToShoot = false;
		reachedDestination = false;
		break;
	}

	state = newState;
}

TRobotState CRobot::getState() {
	return state;
}


/** Return the next destination hex, if any, in this robot's ongoing journey. 
	If the current path is blocked, try to find a way around it. */
//CHex CRobot::getNextTravelHex(CHex& destination) {
//	if (destination == CHex(-1))
//		return destination;
//
//	if (travelPath.empty() || travelPath.back() != destination) {
//		travelPath = world.map->aStarPath(hexPosition, destination);
//		//!!!!check for blocking robots in the first hex and we can exit early
//		if (travelPath.empty()) //no route to destination
//			return CHex(-1);
//	}
//
//	//is previously calculated next destination hex still free ?
//	if (!world.map->isFree(travelPath.front())) {
//		travelPath = world.map->aStarPath(hexPosition, destination, travelPath.front());
//	}
//
//	if (travelPath.empty()) 
//		return CHex(-1);
//
//	CHex hex = travelPath.front();
//	travelPath.erase(travelPath.begin());
//	return hex;
//}

void CRobot::draw() {
	//if (!visibleToPlayer)
	//	return;
	//for (auto hex : viewField.visibleHexes)
	//	hexRendr2.highlightHex(hex);
	CEntity::draw();
}


/** Check if the given segment intersects this robot. */
std::tuple<bool, glm::vec3> CRobot::collisionCheck(glm::vec3& segA, glm::vec3& segB) {
	if (lineModel.BBcollision(segA, segB))
		return { true, glm::vec3() };
	return { false, glm::vec3()};
}

void CRobot::receiveDamage(CEntity& attacker, int damage) {
	//temp!!!!!!!!!!!!!!
	liveLog << "Hit robot!";
	world.deleteEntity(*this);
}



/** Perform realtime melee action. Eg, lunging at target. */
void CRobot::melee3() {

	
	if (lungeState != returning &&
		glm::distance(worldPos, targetEntity->worldPos) > meleeRange + 0.05f) {
		setState(robotCharge3, targetEntity);
		return;
	}


	if (meleeHitCooldown < 1.0f)
		return;

	if (lungeState == preLunge) {
		destination = targetEntity->worldPos;
		lastDistance = glm::distance(worldPos, destination);
		lungeState = lunging;
		speed = 9000.0f;
		reachedDestination = false;
	}

	if (lungeState == lunging) {
		//if (glm::distance(worldPos, destination) > lastDistance) { //overshot
		if (reachedDestination) {
		//	physics.velocity = { 0,0,0 };
			destination = meleeLungeHome;
			lastDistance = FLT_MAX; // glm::distance(worldPos, destination);
			lungeState = returning;
			speed = 9000.0f;
			reachedDestination = false;
		}
	}

	if (lungeState == returning) {
		//if (glm::distance(worldPos, destination) > lastDistance) { //overshot
		if (reachedDestination) {
			//physics.velocity = { 0,0,0 };
			//destination = meleeLungeHome;
			lungeState = preLunge;
			meleeHitCooldown = -1;
			return;
		}
	}

	lastDistance = glm::distance(worldPos, destination);

	//glm::vec3 moveVec = glm::normalize(destination - worldPos);
	//physics.moveImpulse = moveVec * 9000.0f;
}

/** Return true if we can draw a line to the target without hitting anything. */
bool CRobot::hasLineOfSight(CEntity* target) {
	return hasLineOfSight(target->worldPos);
}

bool CRobot::hasLineOfSight(const glm::vec3& p) {
	TIntersections intersectedHexes = world.map->getIntersectedHexes(worldPos, p);
	for (auto& hex : intersectedHexes) {
		if (world.map->getHexCube(hex.first).content != emptyHex)
			return false;
		//TO DO: can expand this to check for other robots blocking
	}

	return true;
}

///** Return true if target within 90 degree field of view and not obscured by scenery.*/
//// TO DO: optimisations include limiting view distance, *not* checking every damn frame, etc.
//bool CRobot::inFoV(CEntity* target) {
//	//range check:
//	if (cubeDistance(target->hexPosition, hexPosition) > 6)
//		return false;
//
//	if ( abs(orientationTo(target->worldPos)) < rad60 /*rad45*/ ) {
//		if (hasLineOfSight(target))
//			return true;
//	}
//	return false;
//}

void CRobot::fireMissile(CEntity* target) {
	auto missile = std::make_shared<CMissile>();

	glm::vec3 targetVec = target->worldPos - worldPos;
	float targetAngle = glm::orientedAngle(glm::normalize(targetVec), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

	missile->setPosition(worldPos, targetAngle);
	missile->setOwner(this);
	world.addSprite(missile);

	snd::play("shoot");
}


void CRobot::wander3() {
	tracking = trackDestination;
	if (reachedDestination) {
		THexList ring = findRing(5, hexPosition);
		CHex randHex;
		int giveUp = 0;
		do {
			randHex = ring[rnd::dice(ring.size()) - 1];
			//can we los randHex?
			glm::vec3 hexWS = cubeToWorldSpace(randHex);
			if (hasLineOfSight(hexWS) ) {
				destination = hexWS;
				reachedDestination = false;
				lastDestinationDist = FLT_MAX;
				//world.map->setHighlight(randHex, 1.0f);
				break;
			}
			giveUp++;

		} while (giveUp < 10); //temp!!!!!!!!!!!

	}

	speed = 1000.0f;
}

/** Charge right at the target entity until we reach it or lose it. */
void CRobot::charge3() {
	tracking = trackTarget;

	//close enough to hit? Switch to melee mode
	if (glm::distance(worldPos, targetEntity->worldPos) < meleeRange + 0.05f) {
		speed = 0;
		reachedDestination = true;
		setState(robotMelee3, targetEntity);
		return;
	}

	//otherwise, chase after
	if (hasLineOfSight(targetEntity)) {
		glm::vec3 chargeVec = targetEntity->worldPos - worldPos;
		float dist = glm::length(chargeVec) - meleeRange;
		chargeVec = glm::normalize(chargeVec) * dist;
		destination = worldPos + chargeVec;
		reachedDestination = false;
		speed = 1500.0f;
	}
	else {
		tracking = trackDestination;
		if (reachedDestination) { //arrived and player not there?
			setState(robotLightSleep);
			return;
		}
	}


	
}


void CRobot::closeAndShoot() {
	tracking = trackTarget;

	//can we see the target?
	if (hasLineOfSight(targetEntity)) {
		if (missileCooldown > 1.0f) {
			fireMissile(targetEntity);
			missileCooldown = 0;
		}

		//should we get closer?
		float targetDist = glm::distance(worldPos, targetEntity->worldPos);

		if (targetDist > idealShootRange && !stoppedToShoot) {
			destination = targetEntity->worldPos;
			speed = 1500.0f;
		}
		else {
			speed = 0;
			stoppedToShoot = true;
			reachedDestination = true;
		}

		if (stoppedToShoot && targetDist > escapeRange) {
			destination = targetEntity->worldPos;
			speed = 1500.0f;
			reachedDestination = false;
		}
	}
	else
		tracking = trackNone;
}



void CRobot::onMovedHex()
{
}

/** Handle the approach to our destination point by slowing down,
	catching overshoot, etc. */
void CRobot::approachDestination() {
	glm::vec3 moveVec = glm::normalize(destination - worldPos);
	physics.moveImpulse = moveVec * speed;


	float dist = glm::distance(worldPos, destination);

	if (dist < 0.05f) {
		physics.velocity = { 0, 0, 0 };
		reachedDestination = true;
		return;
	}

	if (dist < destSlowdownRange) { //time to slow down

		if (dist > lastDestinationDist) { //overshot!
 			physics.velocity = { 0, 0, 0 };
			reachedDestination = true;
			return;
		}

		float p = dist / destSlowdownRange;
		physics.moveImpulse *=  p;
	}

	lastDestinationDist = dist;
}

/** Rotate to stay looking at something. */
void CRobot::track() {
	if (tracking == trackTarget && targetEntity != NULL)
		rotation = glm::orientedAngle(glm::normalize(targetEntity->worldPos - worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

	else if (tracking == trackDestination) 
		rotation = glm::orientedAngle(glm::normalize(destination - worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

}


