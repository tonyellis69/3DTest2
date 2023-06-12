#include "roboState.h"

#include <cmath>
#include <glm/gtx/vector_angle.hpp>

#include <algorithm>

//#include "robot.h"

#include "utils/random.h"
#include "gameState.h"
#include "missile.h"

#include "utils/log.h"
#include "utils/mathsLib.h"

#include "intersect.h"

#include "sound/sound.h"

void CRoboState::update(float dT) {
	this->dT = dT;

	if (trackingState)
		trackTarget();

	updateTreadCycle();

	auto newState = updateState(dT);
	if (newState) {
		//thisEntity->ai = newState;
		int id = newState->getUniqueID();
		thisEntity->ai = newState.get();
		thisEntity->components[id] = newState;

	}
	//FIXME: we're putting this very object out of scope, inside its own function!
	//probably more elegant and safe to create an entity func that we *request*
	//makes the change after we return
}

/** Rotate upper body to track a target, if any. */
void CRoboState::trackTarget() {
	float targetAngle;
	switch (trackingState) {
	case trackEntity:
		targetAngle = shortestAngle(thisEntity->transform->getUpperBodyRotation(), trackingEntity->getPos() - thisEntity->getPos());
		turnUpperBodyTo(targetAngle);
		break;
	case trackPos:

		targetAngle = shortestAngle(thisEntity->transform->getUpperBodyRotation(), trackingPos - thisEntity->getPos());
		turnUpperBodyTo(targetAngle);
		break;
	case trackEnding:
		float dist = fmod(rad360 + thisEntity->transform->getRotation() - thisEntity->transform->getUpperBodyRotation(), rad360);
		float turnStep = dT * upperTurnSpeed;
		if (turnStep < dist) {
			//put in range [-pi - pi] to give turn a direction, ie, clockwise/anti
			if (dist > M_PI)
				turnStep = -turnStep;
			thisEntity->transform->rotateUpperBody(turnStep);
		}
		else {
			thisEntity->transform->setUpperBodyRotation(thisEntity->transform->getRotation());
			trackingState = trackNone;
		}

	}

}

void CRoboState::startTracking(CEntity* target) {
	trackingState = trackEntity;
	trackingEntity = target;
	thisEntity->transform->upperBodyLocked = false;
}

void CRoboState::startTracking(glm::vec3& pos) {
	trackingState = trackPos;
	trackingPos = pos;
	thisEntity->transform->upperBodyLocked = false;
}

void CRoboState::stopTracking() {
	trackingState = trackEnding;
	thisEntity->transform->upperBodyLocked = true;
}

void CRoboState::updateTreadCycle() {
	if (!moving)
		return;

	float treadLoop = 0.3f;

	float velocityMod = 1.0f; //glm::length(phys->velocity) / 1.7f; //1.7f = usual max
	//scaling by velocity doesn't seem to have a visible effect. Shelf for now.

	treadCycle += dT * velocityMod;
	treadCycle = fmod(treadCycle, treadLoop); //loop after travelling treadLoop
	float f = 1.0f - treadCycle / treadLoop; // 0 - 1

	float treadGap = 0.1f;
	treadTranslate = f * treadGap;
	thisEntity->transform->setWalkTranslation(glm::vec3(-treadTranslate, 0, 0));
}

/** Give robot a push toward this destination, which will scale down
	with risk to avoid overshooting. */
glm::vec3 CRoboState::arriveAt(glm::vec3& dest) {
	float proportionalSlowingDist = slowingDist * (chosenSpeed / 1000);
	glm::vec3 targetOffset = dest - thisEntity->transform->worldPos;
	float distance = glm::length(targetOffset);
	float rampedSpeed = chosenSpeed * (distance / proportionalSlowingDist);
	float clippedSpeed = std::min(rampedSpeed, chosenSpeed);
	moving = true;
	return (clippedSpeed / distance) * targetOffset;
}

float CRoboState::speedFor(glm::vec3& dest) {
	float proportionalSlowingDist = slowingDist * (chosenSpeed / 1000);
	glm::vec3 targetOffset = dest - thisEntity->transform->worldPos;
	float distance = glm::length(targetOffset);
	float rampedSpeed = chosenSpeed * (distance / proportionalSlowingDist);
	float clippedSpeed = std::min(rampedSpeed, chosenSpeed);
	moving = true;
	return clippedSpeed;
}

void CRoboState::stopMoving() {
	moving = false;
	thisEntity->phys->moveImpulse = { 0,0,0 };
}


void CRoboState::headTo(glm::vec3& destinationPos) {
	if (backingUp > 0) {
		backingUp -= dT;
		thisEntity->phys->moveImpulse = -thisEntity->transform->getRotationVec() * defaultSpeed;
		return;
	}

	float maxFrameRotation = dT * maxTurnSpeed;
	float desiredTurn = thisEntity->transform->orientationTo(destinationPos);
	float frameRotation = std::copysign(glm::min(maxFrameRotation, abs(desiredTurn)), desiredTurn);
	thisEntity->transform->rotate(frameRotation);

	float speed = speedFor(destinationPos);
	glm::vec3 impulse(0);
	if (abs(desiredTurn) < rad90)
		impulse = thisEntity->transform->getRotationVec() * speed;
	thisEntity->phys->moveImpulse = impulse;

	auto [avoidanceNeeded, risk] = findAvoidance();

	float maxOpposingFrameRotation = maxFrameRotation * 0.5f;


	if (abs(avoidanceNeeded) > 0) {

		//Special case: colliding bots side-by-side heading same way
		if (pRoboCollidee && ((CRoboState*)pRoboCollidee->ai)->backingUp <= 0 &&
			glm::dot(thisEntity->transform->getRotationVec(), pRoboCollidee->transform->getRotationVec()) > 0.8f &&
			glm::distance(thisEntity->getPos(), pRoboCollidee->getPos()) < 1.0f) {
			backingUp = 0.55f;
			return;
		}

		//does avoidance oppose proposed turn?
		if (signbit(avoidanceNeeded) != std::signbit(frameRotation)) {
			float frameAvoidance = std::copysign(glm::min(maxOpposingFrameRotation + abs(frameRotation), abs(avoidanceNeeded)), avoidanceNeeded);
			thisEntity->transform->rotate(frameAvoidance);
		}
		else { //no? work with it
			float frameAvoidance = std::copysign(glm::min(maxFrameRotation, abs(avoidanceNeeded)), avoidanceNeeded);
			if (abs(frameAvoidance) > abs(frameRotation)) {
				float diff = std::copysign(abs(frameAvoidance) - abs(frameRotation), avoidanceNeeded);
				thisEntity->transform->rotate(diff);
			}
		}
	}

	thisEntity->phys->moveImpulse *= 1.0f - risk;
}


/** Return the necessary vector to avoid obstacles ahead. */
//FIXME: ultimately, probably better to set a robot member to the obstacle, so it has all data.
std::tuple<float, float> CRoboState::findAvoidance() {
	glm::vec3 travelDir = thisEntity->transform->getRotationVec();

	pRoboCollidee = nullptr;

	float destinationDist = glm::distance(thisEntity->getPos(), getDestination());
	float farCheckDist = std::min(maxAvoidanceDist, destinationDist);
	float nearCheckDist = 0.3f;
	//0 = steep turns passing obstacle, higher values = stop turning past close obstacles too soon

	if (destinationDist < nearCheckDist) //destination closer than check start? Skip avoidance 
		return { .0f,.0f };

	if (farCheckDist < nearCheckDist) {
		return { .0f,.0f };
	}

	glm::vec3 aheadSegEnd = thisEntity->getPos() + (travelDir * farCheckDist);
	glm::vec3 aheadSegBegin = thisEntity->getPos() + (travelDir * nearCheckDist);
	glm::vec3 aheadSegCentre = aheadSegBegin + (aheadSegEnd - aheadSegBegin) * 0.5f;

	//for graphic debug aid
	tmpAheadVecBegin = aheadSegBegin;
	tmpCollisionPt = glm::vec3(0);
	tmpCollisionSegPt = aheadSegBegin;
	tmpAheadVecEnd = aheadSegEnd;


	std::vector<TObstacle> obstacles = findNearObstacles(aheadSegCentre);
	if (obstacles.empty()) {
		return { .0f,.0f };
	}

	std::sort(obstacles.begin(), obstacles.end(), [&](const auto& a, const auto& b) {
		return glm::distance(thisEntity->getPos(), a.pos) < glm::distance(thisEntity->getPos(), b.pos); });

	auto [obstacle, aheadSegNearestPt] = findCollidable(obstacles, aheadSegBegin, aheadSegEnd);
	if (obstacle.pos == glm::vec3(0))
		return { .0f,.0f };

	tmpCollisionPt = obstacle.pos;
	tmpCollisionSegPt = aheadSegNearestPt;

	//FIXME: temp bodge for when another robot sitting on destination!
	if (glm::distance(getDestination(), obstacle.pos) < 0.2f) {
		//currentState->setDestination(getPos());
		setDestination(thisEntity->getPos());
		return { .0f,.0f };
	}


	//find rotation needed to steer us out of the bounding circle of the obstacle

	glm::vec3 collisionExitVec = aheadSegNearestPt - obstacle.pos;
	glm::vec3 destinationVec = glm::normalize(getDestination() - thisEntity->getPos());
	glm::vec3 destinationNormal = turnRight(destinationVec);

	//which way we turn to avoid depends on whether we're facing the destination or not
	glm::vec3 avoidanceDir;
	if (isAcute(travelDir, destinationVec)) {
		//find what side of destination vector obstacle is, turn other way
		if (isAcute(obstacle.pos - thisEntity->getPos(), destinationNormal))
			avoidanceDir = turnLeft(travelDir);
		else
			avoidanceDir = turnRight(travelDir);
	}
	else {
		//find shortest direction to dest, turn that way.
		if (isAcute(travelDir, destinationNormal))
			avoidanceDir = turnLeft(travelDir);
		else
			avoidanceDir = turnRight(travelDir);
	}

	//if avoidanceDir lies in the direction of collisionExitVec, avoidance lies that way
	float safeDist = obstacle.radius + robotRadius + safeDistAdjust;
	float distanceToSegment = glm::distance(aheadSegNearestPt, obstacle.pos);
	glm::vec3 collisionExitVecN = glm::normalize(collisionExitVec);
	glm::vec3 avoidVec;
	if (isAcute(avoidanceDir, collisionExitVec)) {
		avoidVec = collisionExitVecN * (safeDist - distanceToSegment);
	}
	else { //otherwise, avoidance requires turning the long way past the obstacle
		avoidVec = -collisionExitVecN * (safeDist + distanceToSegment);
	}

	//find rotation required
	glm::vec3 safeVec = glm::normalize(aheadSegNearestPt - thisEntity->getPos() + avoidVec);
	float avoidAngle = glm::orientedAngle(safeVec, travelDir, glm::vec3(0, 0, 1));


	//are we pointing directly enough at the obstacle to want to throttle speed?
	float risk = 0;
	float collisionDist = glm::distance(obstacle.pos, thisEntity->getPos());
	if (collisionDist - obstacle.radius < obstacleProximityLimit) {
		float bearingToObj = glm::dot(travelDir, glm::normalize(obstacle.pos - thisEntity->getPos()));
		risk = glm::smoothstep(obstacleToSide, obstacleAhead, bearingToObj);
	}

	pRoboCollidee = obstacle.bot;
	return { avoidAngle, risk };
}

std::vector<TObstacle> CRoboState::findNearObstacles(glm::vec3& centre) {
	CHex midwayHex = worldSpaceToHex(centre);
	THexList aheadHexes = getNeighbours(midwayHex, 2);

	std::vector<TObstacle> obstacles;

	for (auto& hex : aheadHexes) {
		if (gameWorld.level->getHexArray()->getHexCube(hex).content == solidHex) {
			obstacles.push_back({ cubeToWorldSpace(hex), hexSize * 1.0f });
		}
		else {
			CEntities entities = gameWorld.getEntitiesAt(hex);
			for (auto& entity : entities) {
				if (entity->isRobot && entity != thisEntity) {
					obstacles.push_back({ entity->getPos(),0.8f /* entity->getRadius()*/, entity });
				}
			}
		}
	}

	return obstacles;
}

std::tuple<TObstacle, glm::vec3> CRoboState::findCollidable(std::vector<TObstacle>& obstacles, glm::vec3& aheadSegBegin, glm::vec3& aheadSegEnd) {
	float safeDist;
	float distanceToSegment = 0;
	float obstacleRadius;
	glm::vec3 aheadSegNearestPt;
	for (auto& obstacle : obstacles) {
		//find nearest point on ahead segment to obstacle
		aheadSegNearestPt = closestPointSegment(aheadSegBegin, aheadSegEnd, obstacle.pos);
		if (aheadSegNearestPt == aheadSegBegin)
			continue; //assume obstacle behind us
		distanceToSegment = glm::distance(aheadSegNearestPt, obstacle.pos);
		obstacleRadius = obstacle.radius;
		safeDist = obstacle.radius + robotRadius + safeDistAdjust;
		if (distanceToSegment < safeDist) {
			return { obstacle, aheadSegNearestPt };
		}
	}

	return { TObstacle(), glm::vec3() };
}

/** Check if we're failing to get to our destination, and resolve. */
void CRoboState::amIStuck() {
	stuckCheck += dT;
	if (stuckCheck > 1.0f) {
		stuckCheck = 0;
		float newDist = glm::distance(thisEntity->getPos(), getDestination());
		if (newDist < destinationDist) {
			destinationDist = newDist;
		}
		else {
			destinationDist = FLT_MAX;
			abortDestination();
		}
	}
}

/** What to do when we can't get to the destinatin.*/
void CRoboState::abortDestination() {
	thisEntity->addComponent<CRoboWander>(); //!!!!!!temp!
}

/** Continue turning toward dir, if not facing it. */
bool CRoboState::turnToward(glm::vec3& dir) {
	float maxFrameRotation = dT * maxTurnSpeed;
	float desiredTurn = shortestAngle(thisEntity->transform->getRotationVec(), dir);
	float frameRotation = std::copysign(glm::min(maxFrameRotation, abs(desiredTurn)), desiredTurn);
	if (abs(frameRotation) < 0.001f)
		return false;
	thisEntity->transform->rotate(frameRotation);
	return true;
}

void CRoboState::turnUpperBodyTo(float destAngle) {
	float frameturn = std::copysign(glm::min(upperTurnSpeed, abs(destAngle)), destAngle);
	thisEntity->transform->rotateUpperBody(frameturn);

}

bool CRoboState::inFov(CEntity* target) {
	glm::vec3 targetDir = target->getPos() - thisEntity->getPos();
	targetDir = glm::normalize(targetDir);

	//find upper body rotation as a vector
	glm::vec3 rotVec = thisEntity->transform->getUpperBodyRotationVec();
	if ((glm::dot(rotVec, targetDir)) < cos(rad50))
		return false;

	if (glm::distance(thisEntity->transform->worldPos, target->getPos()) <= 12 &&
		clearLineTo(target->getPos()))
		return true;

	return false;
}

/** Return true if we can draw a line to the target without hitting anything. */
bool CRoboState::clearLineTo(CEntity* target) {
	return clearLineTo(target->getPos());
}

bool CRoboState::clearLineTo(const glm::vec3& p) {
	TIntersections intersectedHexes = getIntersectedHexes(thisEntity->transform->worldPos, p);
	for (auto& hex : intersectedHexes) {
		if (gameWorld.level->getHexArray()->getHexCube(hex.first).content != emptyHex)
			return false;
		//TO DO: can expand this to check for other robots blocking
	}

	return true;
}


bool CRoboState::canSeeEnemy() {
	if (gameWorld.player == nullptr)
		return false;
	return gameWorld.player->visible && !gameWorld.player->playerC->dead && inFov(gameWorld.player);
}


CRoboWander::CRoboWander(CEntity* bot) : CRoboState(bot) {
	chosenSpeed = defaultSpeed;

	THexList ring;
	CHex randHex;
	int dist = 5;
	do {
		int giveUp = 0;
		ring = findRing(dist, thisEntity->transform->hexPosition);
		do {
			randHex = ring[rnd::dice(ring.size()) - 1];
			//can we los randHex?
			glm::vec3 hexWS = cubeToWorldSpace(randHex);
			if (clearLineTo(hexWS)) {
				destination = hexWS;
				turnDestination = glm::orientedAngle(glm::normalize(hexWS - thisEntity->getPos()), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
				lastDestinationDist = FLT_MAX;
				return;
			}
			giveUp++;

		} while (giveUp < 10); //temp!!!!!!!!!!!
		dist--;

	} while (dist > 0);


}


std::shared_ptr<CRoboState> CRoboWander::updateState(float dT) {
	thisEntity->diagnostic += "wandering!";
	if (canSeeEnemy()) {
		if (thisEntity->entityType == entMeleeBot)
			return std::make_shared<CCharge>(thisEntity, gameWorld.player);
		else
			return std::make_shared<CCloseAndShoot>(thisEntity, gameWorld.player);
	}
		//!!!!!!!!!!Temp for testing

	this->dT = dT;

	//proceed to actual movement
	float dist = glm::distance(thisEntity->getPos(), destination);

	if (dist < 0.05f) {
		stopMoving();
		destinationDist = FLT_MAX;
		return std::make_shared<CGlanceAround>(thisEntity); 
	}

	headTo(destination);
	
	return nullptr;
}





CGlanceAround::CGlanceAround(CEntity* bot) : CRoboState(bot) {
	//bot = (CRobot*)thisEntity;
	float leftLimit = fmod(rad360 + thisEntity->transform->getUpperBodyRotation() - rad90, rad360);
	float rightLimit = fmod(rad360+ thisEntity->transform->getUpperBodyRotation() + rad90, rad360);
	cumulativeRotation = 0;
	totalRotation = 0;

	float shortPause = 0.25f;
	float longPause = 0.5f;
	float currentFocus = thisEntity->transform->getUpperBodyRotation();

	thisEntity->transform->upperBodyLocked = false;

	glances = { {currentFocus, shortPause}, {leftLimit, shortPause}, {currentFocus,0}, {rightLimit, shortPause},
		{currentFocus, longPause} };
}

std::shared_ptr<CRoboState> CGlanceAround::updateState(float dT) {
	thisEntity->diagnostic += "glancing!";
	if (canSeeEnemy()) {
		return std::make_shared<CCharge>(thisEntity,gameWorld.player);
		//!!!!!!!!!!Temp for testing!!!!!!!!!!!!!
		thisEntity->transform->upperBodyLocked = true;
		return std::make_shared<CCloseAndShoot>(thisEntity, gameWorld.player);
	}

	if (pause > 0) {
		pause -= dT;
		return nullptr;
	}

	if (glances.empty()) {
		thisEntity->transform->upperBodyLocked = true;
		return std::make_shared<CRoboWander>(thisEntity);
	}

	TGlance dest = glances.front();

	//find shortest angle between dest angle and our direction
	float turnDist; 
	if (totalRotation == 0) {
		turnDist = fmod(rad360 + dest.angle - thisEntity->transform->getUpperBodyRotation(), rad360);
		//put in range [-pi - pi] to give angle a direction, ie, clockwise/anti
		if (turnDist > M_PI)
			turnDist = -(rad360 - turnDist);

		totalRotation = abs(turnDist);
		turnDir = (std::signbit(turnDist)) ? -1.0f : 1.0f;
	}


	float frameTurn = dT * glanceSpeed * turnDir;
	cumulativeRotation += abs(frameTurn);
	if (cumulativeRotation > totalRotation) { //overshot
		thisEntity->transform->setUpperBodyRotation(dest.angle);
		totalRotation = 0;
		cumulativeRotation = 0;
		pause = dest.pause;
		glances.erase(glances.begin());



	}
	else {
		thisEntity->transform->rotateUpperBody(frameTurn);
		//bot->rotation = fmod(rad360 + bot->rotation, rad360); //needed??
	}

	return nullptr;
}


CCharge::CCharge(CEntity* bot, CEntity* targetEntity) : CRoboState(bot) {
	this->targetEntity = targetEntity;
	destination = targetEntity->getPos();
	chosenSpeed = chargeSpeed;
}

std::shared_ptr<CRoboState> CCharge::updateState(float dT) {
	thisEntity->diagnostic += "charging!";
	if (canSeeEnemy()) { //keep destination up to date
		destination = targetEntity->getPos();
		targetInSight = true;
		startTracking(targetEntity);
	}
	else
		targetInSight = false;
		//TO DO: switch to some other behaviour. Prob
		//return std::make_shared<CGoToHunting>(bot, lastSighting, targetEntity);

	//bot->stopMoving();
	//return nullptr;
	
	//reached destination?
	if (glm::distance(thisEntity->getPos(), destination) < meleeRange ) {
		stopMoving();
		if (targetInSight)
			return std::make_shared<CMelee>(thisEntity,targetEntity); 
		else {
			//bot->lineModel.setColourR(glm::vec4(0, 1, 0, 1));
			return std::make_shared<CRoboWander>(thisEntity);
		}
	}

	//otherwise charge at target
	headTo(targetEntity->getPos());
	//bot->diagnostic += "src " + std::to_string(bot->getPos().x) + " "
	//	+ std::to_string(bot->getPos().y) + " dest " +
	//	std::to_string(destination.x) + " " + std::to_string(destination.y);
	// NB src dest remain good numbers throughout charge

	return nullptr;
}




CMelee::CMelee(CEntity* bot, CEntity* targetEntity) : CRoboState(bot) {
	this->targetEntity = targetEntity; 
	lungeVec = glm::normalize(targetEntity->getPos() - thisEntity->getPos());
	timer = 0;
	startPos = thisEntity->getPos();
}

std::shared_ptr<CRoboState> CMelee::updateState(float dT) {
	thisEntity->diagnostic += "meleeing!";
	float targetDist = glm::distance(thisEntity->getPos(), targetEntity->getPos());
	if (targetDist > meleeRange && timer < 0) {
		return std::make_shared<CRoboWander>(thisEntity);
	}

	timer += dT;
	if (timer < 0)
		return nullptr;

	float adj = 0.21f;

	if (timer < lungeEnd) {
		float step = targetDist /(lungeEnd - timer) ;
		step *= dT;
		thisEntity->transform->setPos(thisEntity->getPos() + lungeVec * step * adj);
		return nullptr;
	}

	if (!hit) {
		targetEntity->healthC->receiveDamage(*thisEntity, 10);
		hit = true;
		return nullptr;
	}

	if (timer < returnEnd) {
		float dist = glm::distance(startPos, thisEntity->getPos());
		float step = dist / (returnEnd - timer) ;
		step *= dT;
		thisEntity->transform->setPos( thisEntity->getPos() + -lungeVec * step * adj);
		return nullptr;
	}

	timer = -1.0f;
	hit = false;
	thisEntity->transform->setPos(startPos);

	return nullptr;
}


CCloseAndShoot::CCloseAndShoot(CEntity* bot, CEntity* targetEntity) : CRoboState(bot) {
	//bot->canSeeEnemy = true;
	this->targetEntity = targetEntity;
	startTracking(targetEntity);
	chosenSpeed = maxSpeed; 
}

std::shared_ptr<CRoboState> CCloseAndShoot::updateState(float dT) {
	missileCooldown += dT;
	

	//can we see the target?
	if (clearLineTo(targetEntity)) {
		lastSighting = targetEntity->getPos();

		if (missileCooldown > 1.0f) {
			//((CRobot*)thisEntity)->fireMissile(targetEntity);
			//FIXME: ugh horrible cludge!!!!!!!!!!!!!!!!!!!
			//can prob fire missile here in bot AI

			glm::vec3 targetVec = targetEntity->getPos() - thisEntity->getPos();
			float targetAngle = glm::orientedAngle(glm::normalize(targetVec), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

			auto missile = (CMissile*)gameWorld.spawn("missile", thisEntity->getPos(), targetAngle);
			missile->setOwner(thisEntity); //FIXME: phasing out
			missile->setParent(thisEntity);
			missile->setSpeed(15);// 7.0f);

			snd::play("shoot");


			missileCooldown = 0;
		}

		//should we get closer?
		float targetDist = glm::distance(thisEntity->getPos(), targetEntity->getPos());

		if (targetDist > idealShootRange && !stoppedToShoot) {


			destination = targetEntity->getPos();
			headTo(targetEntity->getPos());

		}
		else {
			stoppedToShoot = true;
			stopMoving();
		}

		if (stoppedToShoot && targetDist > escapeRange) {
			destination = targetEntity->getPos();
			headTo(targetEntity->getPos());

		}
	}
	else { //lost sight of target
		stopTracking(); //!!!Temp! should track lastsighting instead
		return std::make_shared<CGoToHunting>(thisEntity, lastSighting, targetEntity);
	}


	return nullptr;
}




CGoTo::CGoTo(CEntity* bot, glm::vec3& dest) : CRoboState(bot) {
	destination = dest;
	startTracking(dest);
	chosenSpeed = defaultSpeed;
}

std::shared_ptr<CRoboState> CGoTo::updateState(float dT) {
	float dist = glm::distance(thisEntity->getPos(), destination);

	////ensure facing destination
	//float destAngle = glm::orientedAngle(glm::normalize(destination - bot->getPos()), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
	//bot->setRotation(destAngle);


	//if (dist < 0.05f) {
	if (dist < 0.2f) {	//too close causes doubling-back when avoidance is also pushing us away from destination
		stopMoving();
		stopTracking();
		return std::make_shared<CGlanceAround>(thisEntity);
	}

	headTo(destination);

	return nullptr;
}




CGoToHunting::CGoToHunting(CEntity* bot, glm::vec3& dest, CEntity* quarry) : CGoTo(bot, dest) {
	targetEntity = quarry;
	chosenSpeed = defaultSpeed;
}

std::shared_ptr<CRoboState> CGoToHunting::updateState(float dT) {
	//can we see quarry? switch to melee mode - hardcode for now
	if (clearLineTo(targetEntity)) {
		return std::make_shared<CCloseAndShoot>(thisEntity, targetEntity);
	}


	CGoTo::updateState(dT);
	return nullptr;
}

CTurnToSee::CTurnToSee(CEntity* bot, glm::vec3& dir) : CRoboState(bot) {
	this->dir = dir;
	startTracking(thisEntity->getPos() + dir);
}

std::shared_ptr<CRoboState> CTurnToSee::updateState(float dT) {
	if (canSeeEnemy()) {
		return std::make_shared<CCloseAndShoot>(thisEntity, gameWorld.player);
	}

	float success = turnToward(dir);
	if (success)
		return nullptr;
	else
		return std::make_shared<CGlanceAround>(thisEntity);
		//TODO: should be goInvestigate
}

