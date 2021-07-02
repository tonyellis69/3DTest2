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
	viewField.setField(5, 60);

	if (rnd::dice(3) == 1)
		tmpCharger = true;

	isRobot = true;
	physics.invMass = 1.0f / 80.0f; //temp!
}


void CRobot::update(float dT) {
	this->dT = dT;


	if (meleeHitCooldown > 0)
		meleeHitCooldown -= dT;

	if (missileCooldown > 0)
		missileCooldown -= dT;

	if (transitioningToHex) {
		float angleToDest = orientationTo(cubeToWorldSpace(moveDest));
		if (abs(angleToDest) > 0.01f && state != robotEvasiveShoot) { //not lined up, so rotate
			rotateAlong(angleToDest);
		}

		approachDestHex();
	}


	if (inFoV(world.player)) {
		tmpPlayerInFov = true;
	}
	else
		tmpPlayerInFov = false;



	//TO DO: maybe don't try any state updates while transitioning?
	//Almost nothing can start while we're still ending travel to hex
	switch (state) {
	case robotSleep: return;
	case robotWander2 :
		wander2();
		return;
	case robotWander3:
		wander3();
		return;
	case robotCharge:
		charge();
		return;
	case robotLookFor:
		lookFor();
		return;
	case robotLookAround:
		lookAround();
		return;
	case  robotLightSleep:
		if (cubeDistance(hexPosition, world.player->hexPosition) <= 4) {
			setState(robotHunt);
		}
		return;

	case robotChase:
		if ((glm::distance(worldPos, world.player->worldPos) < robotMeleeRange) && meleeHitCooldown <= 0) {
			setState(robotMelee);
			return;
		}

		if (world.player->hexPosition != destination) {
			destination = world.player->hexPosition;
		}

		moveDest = getNextTravelHex(destination);
		if (moveDest == CHex(-1)) {
			return;
		}
		world.map->movingTo(this, hexPosition, moveDest);
		transitioningToHex = true;
		return;

	case robotWander:
		if (!transitioningToHex) {
			moveDest = getNextTravelHex(destination);
			if (moveDest == CHex(-1)) {
				world.map->setHighlight(destination, 0.0f);
				setState(robotSleep);
				return;
			}
			world.map->movingTo(this, hexPosition, moveDest);
			transitioningToHex = true;
		}
		return;

	case robotMelee:
		melee();
		return;

	case robotShoot:
	{
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
		return;
	}

	case robotEvasiveShoot:
		strafe();
		return;

	case robotHunt:
		if (!transitioningToHex) {
			//no longer transitioning, so can we see player from this new hex?
			if (hasLineOfSight(targetEntity)) {
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
			transitioningToHex = true;
		}

	}
}


/** Switch to the given state, performing the necessary initialisation. */
void CRobot::setState(TRobotState newState) 
{
	switch (newState) {
	case robotCharge:
		robotMoveSpeed = 6.0f;
		lineModel.setColourR(hostileColour);
		targetEntity = world.player; //TO DO: don't be so absolute!
		destination = targetEntity->hexPosition; 
		break;
	case robotWander2:
		lineModel.setColourR(glm::vec4(0,1,0,1));
		robotMoveSpeed = 3.0f;
		break;
	case robotWander3:
		lineModel.setColourR(glm::vec4(0, 1, 0, 1));
		//robotMoveSpeed = 3.0f;
		break;
	case robotLookFor:
		lineModel.setColourR(glm::vec4(1, 1, 0, 1));
		destination = targetLastHeading;
		robotMoveSpeed = 6.0f;
		break;
	case robotLookAround:
		rotationSearched = 0;
		break;
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
		meleeHitCooldown = 0.70f;
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
		lineModel.setColourR(shootingColour);
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
	if (destination == CHex(-1))
		return destination;

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
		hexRendr2.highlightHex(hex);
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





/** Move in realtime toward the destination hex, unless we reach it. */
void CRobot::approachDestHex() {
	glm::vec3 dest = cubeToWorldSpace(moveDest);
	glm::vec3 travel = dest - worldPos;
	glm::vec3 moveVec = glm::normalize(travel) * robotMoveSpeed * dT;
	float remainingDist = glm::length(travel);

	if (glm::length(moveVec) > remainingDist) {
		worldPos = dest;
		world.map->movedTo(this, hexPosition, moveDest);
		setPosition(moveDest);
		transitioningToHex = false;
		return;
	}

	worldPos += moveVec;
	buildWorldMatrix();
}

/** Perform as much of the given rotation as we can in this tick. */
void CRobot::rotateAlong(const float& angle) {
	float dRotate = float((angle > 0) - (angle < 0)) * dT * robotRotateSpeed;

	if (abs(dRotate) > abs(angle)) {
		dRotate = angle;
	}

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
bool CRobot::hasLineOfSight(CEntity* target)
{
	TIntersections intersectedHexes = world.map->getIntersectedHexes(worldPos, target->worldPos);
	for (auto& hex : intersectedHexes) {
		if (world.map->getHexCube(hex.first).content != emptyHex)
			return false;
		//TO DO: can expand this to check for other robots blocking
	}

	return true;
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

/** Return true if target within 90 degree field of view and not obscured by scenery.*/
// TO DO: optimisations include limiting view distance, *not* checking every damn frame, etc.
bool CRobot::inFoV(CEntity* target) {
	//range check:
	if (cubeDistance(target->hexPosition, hexPosition) > 6)
		return false;

	if ( abs(orientationTo(target->worldPos)) < rad60 /*rad45*/ ) {
		if (hasLineOfSight(target))
			return true;
	}
	return false;
}

void CRobot::fireMissile(CEntity* target) {
	auto missile = std::make_shared<CMissile>();

	glm::vec3 targetVec = target->worldPos - worldPos;
	float targetAngle = glm::orientedAngle(glm::normalize(targetVec), glm::vec3(1, 0, 0), glm::vec3(0, 0, -1));

	missile->setPosition(worldPos, targetAngle);
	missile->setOwner(this);
	world.addSprite(missile);

	snd::play("shoot");
}

/** Move for several hexes tangentally to target while firing, sometimes closing
	at an angle. */
void CRobot::strafe() {
	if (midRun) {
		if (!transitioningToHex) {
			moveDest = getNextTravelHex(destination);
			if (moveDest == CHex(-1)) { //reached end of run
				midRun = false;
				for (auto& it : pathTemp) {
					world.map->setHighlight(it, 0.0f);
				}

				if (!hasLineOfSight(targetEntity)) {
					setState(robotHunt);
					liveLog << "\nHunting!";
				}
				return;
			}

			world.map->movingTo(this, hexPosition, moveDest);
			transitioningToHex = true;
		}

		//keep facing target
		float angleToFaceTarget = orientationTo(targetEntity->worldPos);
		rotation += angleToFaceTarget;

		//shoot if we can
		if (missileCooldown <= 0) {
			if (hasLineOfSight(targetEntity)) {
				fireMissile(targetEntity);
				missileCooldown = 2;
			}
		}


	}
	else { //start a new run
		CHex startHex;
		THexDir exitDir; glm::vec3 intersection;
		CHex currentHex;
		glm::vec3 targetVec = glm::normalize(targetEntity->worldPos - worldPos);
		float targetAngle = glm::orientedAngle(targetVec, glm::vec3(1, 0, 0), glm::vec3(0, 0, -1));


		

		int approach;
		bool hitSolidHex = false;
		//do 
		{ //explore angle


			//choose whether we're looking for a lateral angle or a closing one.
			approach = rnd::dice(6);
			float r1, r2;
			if (approach < 5) { //lateral
				r1 = M_PI_2; //90 deg from target angle
				r2 = M_PI_2 - 0.523599; //60 deg from target angle
			}
			else { //closing
				r1 = M_PI_2 - 0.523599; //60 deg from target angle
				r2 = 0.523599; //30 deg from target angle
			}

			if (approach % 2 == 0) {
				r1 = -r1; r2 = -r2;
			}
			if (r1 > r2)
				std::swap(r1, r2);

			float randAngle = targetAngle + rnd::rand(r1,r2);

			//attempt to plot line down angle
			glm::vec3 dirVec = glm::rotate(glm::vec3(1, 0, 0), randAngle, glm::vec3(0, 0, 1));
			dirVec *= 5 * hexWidth;
			glm::vec3 endPoint = worldPos + dirVec;
			currentHex = worldSpaceToHex(endPoint);

			TIntersections intersectedHexes = world.map->getIntersectedHexes(worldPos, endPoint);
			//if run ended too short, give up
			for (auto& hex : intersectedHexes) {
				if (world.map->getHexCube(hex.first).content != emptyHex && 
					cubeDistance(hexPosition,hex.first) < 3) {
					hitSolidHex = true;
					return;
				}
			}
		} 

		travelPath = world.map->aStarPath(hexPosition, currentHex);
		for (auto& it : travelPath) {
			world.map->setHighlight(it, 1.0f);
		}
		pathTemp = travelPath;

		destination = travelPath.back();
		midRun = true;

		if (approach < 5)
			liveLog << "\ngoing lateral!";
		else
			liveLog << "\nclosing!";

	}
	
}

void CRobot::wander2() {
	//are we currenly wandering?
	if (!transitioningToHex) {

		if (tmpPlayerInFov) {
			setState(robotCharge);
			return;
		}

		moveDest = getNextTravelHex(destination);
		if (moveDest == CHex(-1)) {

			//for (auto& it : pathTemp) {
			//	world.map->setHighlight(it, 0.0f);
			//}
			//pathTemp.clear();

			
			THexList ring = findRing(5, hexPosition);
			int randHex;
			do {
				randHex = rnd::dice(ring.size()) - 1;
				travelPath = world.map->aStarPath(hexPosition, ring[randHex]);
			} while (travelPath.size() < 3);

			destination = travelPath.back();
			moveDest = travelPath.front();


			//for (auto& it : travelPath) {
			//	world.map->setHighlight(it, 1.0f);
			//}
   //			pathTemp = travelPath;

		}



		world.map->movingTo(this, hexPosition, moveDest);
		transitioningToHex = true;
	}
}

void CRobot::wander3() {
	if (glm::distance(worldPos, destinationWS) < 0.1f || destinationWS == glm::vec3(0, 0, 0)) { //temp!
		THexList ring = findRing(5, hexPosition);
		CHex randHex;
		int giveUp = 0;
		do {
			randHex = ring[rnd::dice(ring.size()) - 1];
			//can we los randHex?
			glm::vec3 hexWS = cubeToWorldSpace(randHex);
			if (hasLineOfSight(hexWS) ) {
				destinationWS = hexWS;
				//world.map->setHighlight(randHex, 1.0f);
				break;
			}
			giveUp++;

		} while (giveUp < 10); //temp!!!!!!!!!!!

	}

	//move toward destination 
	glm::vec3 moveVec = glm::normalize(destinationWS - worldPos);


	float slowDist = 0.3f;
	float dist = glm::distance(destinationWS, worldPos);
	if (dist < slowDist) { //start slowing
		float p = dist / slowDist;
		physics.velocity = glm::mix(glm::vec3(0), physics.velocity, p);
	}
	physics.moveImpulse = moveVec * 500.0f;


	rotation = glm::orientedAngle(glm::normalize(moveVec), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
}

/** Charge right at the target entity until we reach it or lose it. */
void CRobot::charge()  {
	//ensure we're looking at the target:
	float angleToFaceTarget = orientationTo(targetEntity->worldPos);
	rotation += angleToFaceTarget;

	if (!transitioningToHex) { //again
		//is player in vicinity?
		THexList adjacentHexes = getNeighbours(hexPosition);
		for (auto& local : adjacentHexes) {
			if (local == targetEntity->hexPosition) {
				return; //TO DO: switch to melee instead
				if ((glm::distance(worldPos, world.player->worldPos) < robotMeleeRange) && meleeHitCooldown <= 0) {
					setState(robotMelee);
					return;
				}
			}

		}

		//lost sight of player?
		if (!inFoV(targetEntity)) {
			setState(robotLookFor); 
			return;
		}

		//maybe player moved
		if (destination != targetEntity->hexPosition) {
			destination = targetEntity->hexPosition;
		}
		//targetLastHeading = targetEntity->moveDest;
		//bzzzt no longer works with freemoving player

		moveDest = getNextTravelHex(destination);

		world.map->movingTo(this, hexPosition, moveDest);
		transitioningToHex = true;

	}

}

void CRobot::lookFor() {
	if (!transitioningToHex) {
		if (tmpPlayerInFov) {
			setState(robotCharge);
			return;
		}

		moveDest = getNextTravelHex(destination);
		if (moveDest == CHex(-1)) {
			setState(robotLookAround); 
			return;
		}

		world.map->movingTo(this, hexPosition, moveDest);
		transitioningToHex = true;
	}
}


void CRobot::lookAround() {
	if (tmpPlayerInFov) {
		setState(robotCharge);
		return;
	}

	if (rotationSearched > 2 * M_PI) {
		setState(robotWander2);
		return;
	}

	float dTurn = dT * robotRotateSpeed;
	rotation += dTurn;
	rotationSearched += dTurn;
	buildWorldMatrix();
}

void CRobot::onMovedHex()
{
}


