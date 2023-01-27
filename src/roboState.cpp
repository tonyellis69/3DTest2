#include "roboState.h"

#include <cmath>
#include <glm/gtx/vector_angle.hpp>

#include <algorithm>

#include "robot.h"

#include "utils/random.h"
#include "gameState.h"

#include "utils/log.h"
#include "utils/mathsLib.h"

#include "intersect.h"


void CRoboState::update(float dT) {
	this->dT = dT;

	if (trackingState)
		trackTarget();

	updateTreadCycle();

	auto newState = updateState(dT);
	if (newState)
		pBot->ai = newState;
	//FIXME: we're putting this very object out of scope, inside its own function!
	//probably more elegant and safe to create an entity func that we *request*
	//makes the change after we return
}

/** Rotate upper body to track a target, if any. */
void CRoboState::trackTarget() {
	float targetAngle;
	switch (trackingState) {
	case trackEntity:
		targetAngle = shortestAngle(pBot->transform->getUpperBodyRotation(), trackingEntity->getPos() - pBot->getPos());
		turnUpperBodyTo(targetAngle);
		break;
	case trackPos:

		targetAngle = shortestAngle(pBot->transform->getUpperBodyRotation(), trackingPos - pBot->getPos());
		turnUpperBodyTo(targetAngle);
		break;
	case trackEnding:
		float dist = fmod(rad360 + pBot->transform->getRotation() - pBot->transform->getUpperBodyRotation(), rad360);
		float turnStep = dT * upperTurnSpeed;
		if (turnStep < dist) {
			//put in range [-pi - pi] to give turn a direction, ie, clockwise/anti
			if (dist > M_PI)
				turnStep = -turnStep;
			pBot->transform->rotateUpperBody(turnStep);
		}
		else {
			pBot->transform->setUpperBodyRotation(pBot->transform->getRotation());
			trackingState = trackNone;
		}

	}

}

void CRoboState::startTracking(CEntity* target) {
	trackingState = trackEntity;
	trackingEntity = target;
	pBot->transform->upperBodyLocked = false;
}

void CRoboState::startTracking(glm::vec3& pos) {
	trackingState = trackPos;
	trackingPos = pos;
	pBot->transform->upperBodyLocked = false;
}

void CRoboState::stopTracking() {
	trackingState = trackEnding;
	pBot->transform->upperBodyLocked = true;
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
	pBot->transform->setWalkTranslation(glm::vec3(-treadTranslate, 0, 0));
}

/** Give robot a push toward this destination, which will scale down
	with risk to avoid overshooting. */
glm::vec3 CRoboState::arriveAt(glm::vec3& dest) {
	float proportionalSlowingDist = slowingDist * (chosenSpeed / 1000);
	glm::vec3 targetOffset = dest - pBot->transform->worldPos;
	float distance = glm::length(targetOffset);
	float rampedSpeed = chosenSpeed * (distance / proportionalSlowingDist);
	float clippedSpeed = std::min(rampedSpeed, chosenSpeed);
	moving = true;
	return (clippedSpeed / distance) * targetOffset;
}

float CRoboState::speedFor(glm::vec3& dest) {
	float proportionalSlowingDist = slowingDist * (chosenSpeed / 1000);
	glm::vec3 targetOffset = dest - pBot->transform->worldPos;
	float distance = glm::length(targetOffset);
	float rampedSpeed = chosenSpeed * (distance / proportionalSlowingDist);
	float clippedSpeed = std::min(rampedSpeed, chosenSpeed);
	moving = true;
	return clippedSpeed;
}

void CRoboState::stopMoving() {
	moving = false;
	pBot->phys->moveImpulse = { 0,0,0 };
}


void CRoboState::headTo(glm::vec3& destinationPos) {
	if (backingUp > 0) {
		backingUp -= dT;
		pBot->phys->moveImpulse = -pBot->transform->getRotationVec() * defaultSpeed;
		return;
	}

	float maxFrameRotation = dT * maxTurnSpeed;
	float desiredTurn = pBot->transform->orientationTo(destinationPos);
	float frameRotation = std::copysign(glm::min(maxFrameRotation, abs(desiredTurn)), desiredTurn);
	pBot->transform->rotate(frameRotation);

	float speed = speedFor(destinationPos);
	glm::vec3 impulse(0);
	if (abs(desiredTurn) < rad90)
		impulse = pBot->transform->getRotationVec() * speed;
	pBot->phys->moveImpulse = impulse;

	auto [avoidanceNeeded, risk] = findAvoidance();

	float maxOpposingFrameRotation = maxFrameRotation * 0.5f;


	if (abs(avoidanceNeeded) > 0) {

		//Special case: colliding bots side-by-side heading same way
		if (pRoboCollidee && ((CRoboState*)pRoboCollidee->ai.get())->backingUp <= 0 &&
			glm::dot(pBot->transform->getRotationVec(), pRoboCollidee->transform->getRotationVec()) > 0.8f &&
			glm::distance(pBot->getPos(), pRoboCollidee->getPos()) < 1.0f) {
			backingUp = 0.55f;
			return;
		}

		//does avoidance oppose proposed turn?
		if (signbit(avoidanceNeeded) != std::signbit(frameRotation)) {
			float frameAvoidance = std::copysign(glm::min(maxOpposingFrameRotation + abs(frameRotation), abs(avoidanceNeeded)), avoidanceNeeded);
			pBot->transform->rotate(frameAvoidance);
		}
		else { //no? work with it
			float frameAvoidance = std::copysign(glm::min(maxFrameRotation, abs(avoidanceNeeded)), avoidanceNeeded);
			if (abs(frameAvoidance) > abs(frameRotation)) {
				float diff = std::copysign(abs(frameAvoidance) - abs(frameRotation), avoidanceNeeded);
				pBot->transform->rotate(diff);
			}
		}
	}

	pBot->phys->moveImpulse *= 1.0f - risk;
}


/** Return the necessary vector to avoid obstacles ahead. */
//FIXME: ultimately, probably better to set a robot member to the obstacle, so it has all data.
std::tuple<float, float> CRoboState::findAvoidance() {
	glm::vec3 travelDir = pBot->transform->getRotationVec();

	pRoboCollidee = nullptr;

	float destinationDist = glm::distance(pBot->getPos(), getDestination());
	float farCheckDist = std::min(maxAvoidanceDist, destinationDist);
	float nearCheckDist = 0.3f;
	//0 = steep turns passing obstacle, higher values = stop turning past close obstacles too soon

	if (destinationDist < nearCheckDist) //destination closer than check start? Skip avoidance 
		return { .0f,.0f };

	if (farCheckDist < nearCheckDist) {
		return { .0f,.0f };
	}

	glm::vec3 aheadSegEnd = pBot->getPos() + (travelDir * farCheckDist);
	glm::vec3 aheadSegBegin = pBot->getPos() + (travelDir * nearCheckDist);
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
		return glm::distance(pBot->getPos(), a.pos) < glm::distance(pBot->getPos(), b.pos); });

	auto [obstacle, aheadSegNearestPt] = findCollidable(obstacles, aheadSegBegin, aheadSegEnd);
	if (obstacle.pos == glm::vec3(0))
		return { .0f,.0f };

	tmpCollisionPt = obstacle.pos;
	tmpCollisionSegPt = aheadSegNearestPt;

	//FIXME: temp bodge for when another robot sitting on destination!
	if (glm::distance(getDestination(), obstacle.pos) < 0.2f) {
		//currentState->setDestination(getPos());
		setDestination(pBot->getPos());
		return { .0f,.0f };
	}


	//find rotation needed to steer us out of the bounding circle of the obstacle

	glm::vec3 collisionExitVec = aheadSegNearestPt - obstacle.pos;
	glm::vec3 destinationVec = glm::normalize(getDestination() - pBot->getPos());
	glm::vec3 destinationNormal = turnRight(destinationVec);

	//which way we turn to avoid depends on whether we're facing the destination or not
	glm::vec3 avoidanceDir;
	if (isAcute(travelDir, destinationVec)) {
		//find what side of destination vector obstacle is, turn other way
		if (isAcute(obstacle.pos - pBot->getPos(), destinationNormal))
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
	glm::vec3 safeVec = glm::normalize(aheadSegNearestPt - pBot->getPos() + avoidVec);
	float avoidAngle = glm::orientedAngle(safeVec, travelDir, glm::vec3(0, 0, 1));


	//are we pointing directly enough at the obstacle to want to throttle speed?
	float risk = 0;
	float collisionDist = glm::distance(obstacle.pos, pBot->getPos());
	if (collisionDist - obstacle.radius < obstacleProximityLimit) {
		float bearingToObj = glm::dot(travelDir, glm::normalize(obstacle.pos - pBot->getPos()));
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
		if (game.map->getHexArray()->getHexCube(hex).content == solidHex) {
			obstacles.push_back({ cubeToWorldSpace(hex), hexSize * 1.0f });
		}
		else {
			CEntities entities = game.map->getEntitiesAt(hex);
			for (auto& entity : entities) {
				if (entity->isRobot && entity != pBot) {
					obstacles.push_back({ entity->getPos(),0.8f /* entity->getRadius()*/, (CRobot*)entity });
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
		float newDist = glm::distance(pBot->getPos(), getDestination());
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
	pBot->ai = std::make_shared<CRoboWander>(pBot); //!!!!!!temp!
}

/** Continue turning toward dir, if not facing it. */
bool CRoboState::turnToward(glm::vec3& dir) {
	float maxFrameRotation = dT * maxTurnSpeed;
	float desiredTurn = shortestAngle(pBot->transform->getRotationVec(), dir);
	float frameRotation = std::copysign(glm::min(maxFrameRotation, abs(desiredTurn)), desiredTurn);
	if (abs(frameRotation) < 0.001f)
		return false;
	pBot->transform->rotate(frameRotation);
	return true;
}

void CRoboState::turnUpperBodyTo(float destAngle) {
	float frameturn = std::copysign(glm::min(upperTurnSpeed, abs(destAngle)), destAngle);
	pBot->transform->rotateUpperBody(frameturn);

}

bool CRoboState::inFov(CEntity* target) {
	glm::vec3 targetDir = target->getPos() - pBot->getPos();
	targetDir = glm::normalize(targetDir);

	//find upper body rotation as a vector
	glm::vec3 rotVec = pBot->transform->getUpperBodyRotationVec();
	if ((glm::dot(rotVec, targetDir)) < cos(rad50))
		return false;

	if (glm::distance(pBot->transform->worldPos, target->getPos()) <= 12 &&
		clearLineTo(target->getPos()))
		return true;

	return false;
}

/** Return true if we can draw a line to the target without hitting anything. */
bool CRoboState::clearLineTo(CEntity* target) {
	return clearLineTo(target->getPos());
}

bool CRoboState::clearLineTo(const glm::vec3& p) {
	TIntersections intersectedHexes = getIntersectedHexes(pBot->transform->worldPos, p);
	for (auto& hex : intersectedHexes) {
		if (game.map->getHexArray()->getHexCube(hex.first).content != emptyHex)
			return false;
		//TO DO: can expand this to check for other robots blocking
	}

	return true;
}


bool CRoboState::canSeeEnemy() {
	if (game.player == nullptr)
		return false;
	return game.player->visible && !game.player->dead && inFov(game.player);
}


CRoboWander::CRoboWander(CRobot* bot) : CRoboState(bot) {
	chosenSpeed = defaultSpeed;

	THexList ring;
	CHex randHex;
	int dist = 5;
	do {
		int giveUp = 0;
		ring = findRing(dist, pBot->transform->hexPosition);
		do {
			randHex = ring[rnd::dice(ring.size()) - 1];
			//can we los randHex?
			glm::vec3 hexWS = cubeToWorldSpace(randHex);
			if (clearLineTo(hexWS)) {
				destination = hexWS;
				turnDestination = glm::orientedAngle(glm::normalize(hexWS - pBot->getPos()), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
				lastDestinationDist = FLT_MAX;
				return;
			}
			giveUp++;

		} while (giveUp < 10); //temp!!!!!!!!!!!
		dist--;

	} while (dist > 0);


}


std::shared_ptr<CRoboState> CRoboWander::updateState(float dT) {
	pBot->diagnostic += "wandering!";
	if (canSeeEnemy()) {
		if (pBot->entityType == entMeleeBot)
			return std::make_shared<CCharge>(pBot, game.player);
		else
			return std::make_shared<CCloseAndShoot>(pBot, game.player);
	}
		//!!!!!!!!!!Temp for testing

	this->dT = dT;

	//proceed to actual movement
	float dist = glm::distance(pBot->getPos(), destination);

	if (dist < 0.05f) {
		stopMoving();
		destinationDist = FLT_MAX;
		return std::make_shared<CGlanceAround>(pBot); 
	}

	headTo(destination);
	
	return nullptr;
}





CGlanceAround::CGlanceAround(CRobot* bot) : CRoboState(bot) {
	//bot = (CRobot*)parentEntity;
	float leftLimit = fmod(rad360 + pBot->transform->getUpperBodyRotation() - rad90, rad360);
	float rightLimit = fmod(rad360+ pBot->transform->getUpperBodyRotation() + rad90, rad360);
	cumulativeRotation = 0;
	totalRotation = 0;

	float shortPause = 0.25f;
	float longPause = 0.5f;
	float currentFocus = pBot->transform->getUpperBodyRotation();

	pBot->transform->upperBodyLocked = false;

	glances = { {currentFocus, shortPause}, {leftLimit, shortPause}, {currentFocus,0}, {rightLimit, shortPause},
		{currentFocus, longPause} };
}

std::shared_ptr<CRoboState> CGlanceAround::updateState(float dT) {
	pBot->diagnostic += "glancing!";
	if (canSeeEnemy()) {
		return std::make_shared<CCharge>(pBot,game.player);
		//!!!!!!!!!!Temp for testing!!!!!!!!!!!!!
		pBot->transform->upperBodyLocked = true;
		return std::make_shared<CCloseAndShoot>(pBot, game.player);
	}

	if (pause > 0) {
		pause -= dT;
		return nullptr;
	}

	if (glances.empty()) {
		pBot->transform->upperBodyLocked = true;
		return std::make_shared<CRoboWander>(pBot);
	}

	TGlance dest = glances.front();

	//find shortest angle between dest angle and our direction
	float turnDist; 
	if (totalRotation == 0) {
		turnDist = fmod(rad360 + dest.angle - pBot->transform->getUpperBodyRotation(), rad360);
		//put in range [-pi - pi] to give angle a direction, ie, clockwise/anti
		if (turnDist > M_PI)
			turnDist = -(rad360 - turnDist);

		totalRotation = abs(turnDist);
		turnDir = (std::signbit(turnDist)) ? -1.0f : 1.0f;
	}


	float frameTurn = dT * glanceSpeed * turnDir;
	cumulativeRotation += abs(frameTurn);
	if (cumulativeRotation > totalRotation) { //overshot
		pBot->transform->setUpperBodyRotation(dest.angle);
		totalRotation = 0;
		cumulativeRotation = 0;
		pause = dest.pause;
		glances.erase(glances.begin());



	}
	else {
		pBot->transform->rotateUpperBody(frameTurn);
		//bot->rotation = fmod(rad360 + bot->rotation, rad360); //needed??
	}

	return nullptr;
}


CCharge::CCharge(CRobot* bot, CEntity* targetEntity) : CRoboState(bot) {
	this->targetEntity = targetEntity;
	destination = targetEntity->getPos();
	chosenSpeed = chargeSpeed;
}

std::shared_ptr<CRoboState> CCharge::updateState(float dT) {
	pBot->diagnostic += "charging!";
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
	if (glm::distance(pBot->getPos(), destination) < meleeRange ) {
		stopMoving();
		if (targetInSight)
			return std::make_shared<CMelee>(pBot,targetEntity); 
		else {
			//bot->lineModel.setColourR(glm::vec4(0, 1, 0, 1));
			return std::make_shared<CRoboWander>(pBot);
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




CMelee::CMelee(CRobot* bot, CEntity* targetEntity) : CRoboState(bot) {
	this->targetEntity = targetEntity; 
	lungeVec = glm::normalize(targetEntity->getPos() - pBot->getPos());
	timer = 0;
	startPos = pBot->getPos();
}

std::shared_ptr<CRoboState> CMelee::updateState(float dT) {
	pBot->diagnostic += "meleeing!";
	float targetDist = glm::distance(pBot->getPos(), targetEntity->getPos());
	if (targetDist > meleeRange && timer < 0) {
		return std::make_shared<CRoboWander>(pBot);
	}

	timer += dT;
	if (timer < 0)
		return nullptr;

	float adj = 0.21f;

	if (timer < lungeEnd) {
		float step = targetDist /(lungeEnd - timer) ;
		step *= dT;
		pBot->getPos() += lungeVec * step * adj;
		return nullptr;
	}

	if (!hit) {
		targetEntity->receiveDamage(*pBot, 10);
		hit = true;
		return nullptr;
	}

	if (timer < returnEnd) {
		float dist = glm::distance(startPos, pBot->getPos());
		float step = dist / (returnEnd - timer) ;
		step *= dT;
		pBot->transform->setPos( pBot->getPos() + -lungeVec * step * adj);
		return nullptr;
	}

	timer = -1.0f;
	hit = false;
	pBot->transform->setPos(startPos);

	return nullptr;
}


CCloseAndShoot::CCloseAndShoot(CRobot* bot, CEntity* targetEntity) : CRoboState(bot) {
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
			pBot->fireMissile(targetEntity);
			missileCooldown = 0;
		}

		//should we get closer?
		float targetDist = glm::distance(pBot->getPos(), targetEntity->getPos());

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
		return std::make_shared<CGoToHunting>(pBot, lastSighting, targetEntity);
	}


	return nullptr;
}




CGoTo::CGoTo(CRobot* bot, glm::vec3& dest) : CRoboState(bot) {
	destination = dest;
	startTracking(dest);
	chosenSpeed = defaultSpeed;
}

std::shared_ptr<CRoboState> CGoTo::updateState(float dT) {
	float dist = glm::distance(pBot->getPos(), destination);

	////ensure facing destination
	//float destAngle = glm::orientedAngle(glm::normalize(destination - bot->getPos()), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
	//bot->setRotation(destAngle);


	//if (dist < 0.05f) {
	if (dist < 0.2f) {	//too close causes doubling-back when avoidance is also pushing us away from destination
		stopMoving();
		stopTracking();
		return std::make_shared<CGlanceAround>(pBot);
	}

	headTo(destination);

	return nullptr;
}




CGoToHunting::CGoToHunting(CRobot* bot, glm::vec3& dest, CEntity* quarry) : CGoTo(bot, dest) {
	targetEntity = quarry;
	chosenSpeed = defaultSpeed;
}

std::shared_ptr<CRoboState> CGoToHunting::updateState(float dT) {
	//can we see quarry? switch to melee mode - hardcode for now
	if (clearLineTo(targetEntity)) {
		return std::make_shared<CCloseAndShoot>(pBot, targetEntity);
	}


	CGoTo::updateState(dT);
	return nullptr;
}

CTurnToSee::CTurnToSee(CRobot* bot, glm::vec3& dir) : CRoboState(bot) {
	this->dir = dir;
	startTracking(pBot->getPos() + dir);
}

std::shared_ptr<CRoboState> CTurnToSee::updateState(float dT) {
	if (canSeeEnemy()) {
		return std::make_shared<CCloseAndShoot>(pBot, game.player);
	}

	float success = turnToward(dir);
	if (success)
		return nullptr;
	else
		return std::make_shared<CGlanceAround>(pBot);
		//TODO: should be goInvestigate
}

