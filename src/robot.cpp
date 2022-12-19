#include "robot.h"

#include <cmath>
#include <glm/gtx/rotate_vector.hpp>


#include "utils/log.h"

#include "hexRenderer.h"

#include "gameState.h"
#include "missile.h"

#include <glm/gtx/vector_angle.hpp>

#include "sound/sound.h"

#include "utils/random.h"

#include "spawner.h"

#include "renderer/imRendr/imRendr.h"

#include "intersect.h"
#include "utils/mathsLib.h"


const float rad90 = float(M_PI) / 2;
const float rad80 = 1.39626f;
const float rad70 = 1.22173f;
const float rad60 = float(M_PI) / 3;
const float rad50 = 0.872665f;
const float rad45 = float(M_PI) / 4;
const float rad40 = 0.698132f;
const float rad30 = float(M_PI) / 6;
const float rad120 = rad360 / 3;

CRobot::CRobot() {

	isRobot = true;
	physics.invMass = 1.0f / 80.0f; //temp!
}




void CRobot::update(float dT) {
	CEntity::update(dT);

	this->dT = dT;
	diagnostic = "";

	if (currentState) {
		auto newState = currentState->update(dT);
		if (newState)
			currentState = newState;
	}



	if (trackingState)
		trackTarget();

	updateTreadCycle();

	//if (moving) 
		//amIStuck(); //FIX: crudely geared to destination only
	//FIX: temp removed to track bugs

	buildWorldMatrix();
}


/** Switch to the given state, performing the necessary initialisation. */
void CRobot::setState(TRobotState newState, CEntity* entity)
{
	switch (newState) {
	case robotWander3:
		currentState = std::make_shared<CRoboWander>(this);
		//lineModel.setColourR(glm::vec4(0, 1, 0, 1)); //move inside!
		break;
	case robotDoNothing:
		currentState = std::make_shared<CDoNothing>(this);
	}

}



//void CRobot::rotate(float angle) {
//	rotation += angle;
//	rotation = fmod(rotation + rad360, rad360);
//	if (transform->upperBodyLocked) {
//		upperBodyRotation += angle;
//		upperBodyRotation = fmod(upperBodyRotation + rad360, rad360);
//	}
//}

//void CRobot::setUpperRotation(float angle) {
//	upperBodyRotation = angle;
//}
//
//void CRobot::rotateUpper(float angle) {
//	upperBodyRotation += angle;
//	upperBodyRotation = fmod(upperBodyRotation + rad360, rad360);
//}





/** Check if the given segment intersects this robot. */
std::tuple<bool, glm::vec3> CRobot::collisionCheck(glm::vec3& segA, glm::vec3& segB) {
	if (modelCmp->model.BBcollision(segA, segB))
		return { true, glm::vec3() };
	return { false, glm::vec3()};
}

void CRobot::buildWorldMatrix() {
	return;

}

void CRobot::startTracking(CEntity* target) {
	trackingState = trackEntity;
	trackingEntity = target;
	transform->upperBodyLocked = false;
}

void CRobot::startTracking(glm::vec3& pos) {
	trackingState = trackPos;
	trackingPos = pos;
	transform->upperBodyLocked = false;
}





void CRobot::stopTracking() {
	trackingState = trackEnding;
	transform->upperBodyLocked = true;
}



void CRobot::receiveDamage(CEntity& attacker, int damage) {
	hp--;
	if (hp == 0) {
		game.killEntity(*this);
		spawn::explosion("explosion", getPos(), 1.5f);
		toRemove = true;
	}
	else {
		if (canSeeEnemy() == false) {
			currentState = std::make_shared<CTurnToSee>(this, glm::normalize(attacker.getPos() - getPos()));
		}
	}
}

/** Give robot a push toward this destination, which will scale down
	with risk to avoid overshooting. */
glm::vec3 CRobot::arriveAt(glm::vec3& dest) {
	float proportionalSlowingDist = slowingDist * (chosenSpeed / 1000);
	glm::vec3 targetOffset = dest - transform->worldPos;
	float distance = glm::length(targetOffset);
	float rampedSpeed = chosenSpeed * (distance / proportionalSlowingDist);
	float clippedSpeed = std::min(rampedSpeed, chosenSpeed);
	moving = true;
	return (clippedSpeed / distance) * targetOffset;
}

float CRobot::speedFor(glm::vec3& dest) {
	float proportionalSlowingDist = slowingDist * (chosenSpeed / 1000);
	glm::vec3 targetOffset = dest - transform->worldPos;
	float distance = glm::length(targetOffset);
	float rampedSpeed = chosenSpeed * (distance / proportionalSlowingDist);
	float clippedSpeed = std::min(rampedSpeed, chosenSpeed);
	moving = true;
	return clippedSpeed;
}


/** Return true if we can draw a line to the target without hitting anything. */
bool CRobot::clearLineTo(CEntity* target) {
	return clearLineTo(target->getPos());
}

bool CRobot::clearLineTo(const glm::vec3& p) {
	TIntersections intersectedHexes = getIntersectedHexes(transform->worldPos, p);
	for (auto& hex : intersectedHexes) {
		if (game.map->getHexArray()->getHexCube(hex.first).content != emptyHex)
			return false;
		//TO DO: can expand this to check for other robots blocking
	}

	return true;
}


bool CRobot::inFov(CEntity* target) {
	glm::vec3 targetDir = target->getPos() - getPos();
	targetDir = glm::normalize(targetDir);

	//find upper body rotation as a vector
	glm::vec3 rotVec = transform->getUpperBodyRotationVec();
	if ((glm::dot(rotVec, targetDir)) < cos(rad50))
		return false;

	if ( glm::distance(transform->worldPos, target->getPos()) <= 12 && 
		clearLineTo(target->getPos()) )
			return true;
	
	return false;
}

std::vector<TObstacle> CRobot::findNearObstacles(glm::vec3& centre) {
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
				if (entity->isRobot && entity != this) {
					obstacles.push_back({ entity->getPos(),0.8f /* entity->getRadius()*/, (CRobot*)entity});
				}
			}
		}
	}

	return obstacles;
}


/** Check if we're failing to get to our destination, and resolve. */
void CRobot::amIStuck() {
	stuckCheck += dT;
	if (stuckCheck > 1.0f) {
		stuckCheck = 0;
		float newDist = glm::distance(getPos(), getDestination());
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
void CRobot::abortDestination() {
	currentState = std::make_shared<CRoboWander>(this); //!!!!!!temp!
}

void CRobot::fireMissile(CEntity* target) {
	glm::vec3 targetVec = target->getPos() - getPos();
	float targetAngle = glm::orientedAngle(glm::normalize(targetVec), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

	auto missile = (CMissile*) spawn::missile("missile",getPos(), targetAngle).get();
	missile->setOwner(this);
	missile->setSpeed(15);// 7.0f);

	snd::play("shoot");
}


void CRobot::onMovedHex()
{
}

/** Continue turning toward dir, if not facing it. */
bool CRobot::turnToward(glm::vec3& dir) {
	float maxFrameRotation = dT * maxTurnSpeed;
	float desiredTurn = shortestAngle(transform->getRotationVec(), dir);
	float frameRotation = std::copysign(glm::min(maxFrameRotation, abs(desiredTurn)), desiredTurn);
	if (abs(frameRotation) < 0.001f)
		return false;
	transform->rotate(frameRotation);
	return true;
}

void CRobot::stopMoving() {
	moving = false;
	physics.moveImpulse = { 0,0,0 };
}

glm::vec3 CRobot::getDestination() {
	return currentState->getDestination();
}

/** Return the necessary vector to avoid obstacles ahead. */
//FIXME: ultimately, probably better to set a robot member to the obstacle, so it has all data.
std::tuple<float, float> CRobot::findAvoidance() {
	glm::vec3 travelDir = transform->getRotationVec();

	pRoboCollidee = nullptr;

	float destinationDist = glm::distance(getPos(), getDestination());
	float farCheckDist = std::min(maxAvoidanceDist, destinationDist) ;
	float nearCheckDist = 0.3f;
	//0 = steep turns passing obstacle, higher values = stop turning past close obstacles too soon

	if (destinationDist < nearCheckDist) //destination closer than check start? Skip avoidance 
		return { .0f,.0f };

	if (farCheckDist < nearCheckDist) {
		return { .0f,.0f };
	}

	glm::vec3 aheadSegEnd = getPos() + (travelDir * farCheckDist);
	glm::vec3 aheadSegBegin = getPos() + (travelDir * nearCheckDist);
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
		return glm::distance(getPos(), a.pos) < glm::distance(getPos(), b.pos); });

	auto [obstacle, aheadSegNearestPt] = findCollidable(obstacles, aheadSegBegin, aheadSegEnd);
	if (obstacle.pos == glm::vec3(0)) 
		return { .0f,.0f };

	tmpCollisionPt = obstacle.pos;
	tmpCollisionSegPt = aheadSegNearestPt;

	//FIXME: temp bodge for when another robot sitting on destination!
	if (glm::distance(getDestination(), obstacle.pos) < 0.2f) {
		currentState->setDestination(getPos());
		return { .0f,.0f };
	}


	//find rotation needed to steer us out of the bounding circle of the obstacle

 	glm::vec3 collisionExitVec = aheadSegNearestPt - obstacle.pos;
	glm::vec3 destinationVec = glm::normalize(getDestination() - getPos());
	glm::vec3 destinationNormal = turnRight(destinationVec);
	
	//which way we turn to avoid depends on whether we're facing the destination or not
	glm::vec3 avoidanceDir;
	if (isAcute(travelDir, destinationVec) ) {
		//find what side of destination vector obstacle is, turn other way
		if (isAcute(obstacle.pos - getPos(), destinationNormal))
			avoidanceDir = turnLeft(travelDir);
		else
			avoidanceDir = turnRight(travelDir);
	}	
	else {  
		//find shortest direction to dest, turn that way.
		if (isAcute(travelDir, destinationNormal) )
			avoidanceDir = turnLeft(travelDir);
		else
			avoidanceDir = turnRight(travelDir);
	}

	//if avoidanceDir lies in the direction of collisionExitVec, avoidance lies that way
	float safeDist = obstacle.radius + robotRadius + safeDistAdjust;
	float distanceToSegment = glm::distance(aheadSegNearestPt, obstacle.pos);
	glm::vec3 collisionExitVecN = glm::normalize(collisionExitVec);
	glm::vec3 avoidVec;
	if (isAcute(avoidanceDir, collisionExitVec) ) {
		avoidVec = collisionExitVecN * (safeDist - distanceToSegment);
	}
	else { //otherwise, avoidance requires turning the long way past the obstacle
		avoidVec = -collisionExitVecN * (safeDist + distanceToSegment);
	}

	//find rotation required
	glm::vec3 safeVec = glm::normalize(aheadSegNearestPt - getPos() + avoidVec);
	float avoidAngle = glm::orientedAngle(safeVec, travelDir, glm::vec3(0, 0, 1));

		
	//are we pointing directly enough at the obstacle to want to throttle speed?
	float risk = 0;
	float collisionDist = glm::distance(obstacle.pos, getPos());
	if (collisionDist - obstacle.radius < obstacleProximityLimit) { 
		float bearingToObj = glm::dot(travelDir, glm::normalize(obstacle.pos - getPos()));
		risk = glm::smoothstep(obstacleToSide, obstacleAhead, bearingToObj);
	}

	pRoboCollidee = obstacle.bot;
	return { avoidAngle, risk };
}

std::tuple<TObstacle, glm::vec3> CRobot::findCollidable(std::vector<TObstacle>& obstacles, glm::vec3& aheadSegBegin, glm::vec3& aheadSegEnd) {
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
			return {obstacle, aheadSegNearestPt};
		}
	}

	return { TObstacle(), glm::vec3() };
}


void CRobot::headTo(glm::vec3 & destinationPos) {
	if (backingUp > 0) {
		backingUp -= dT;
		physics.moveImpulse = -transform->getRotationVec() * defaultSpeed ;
		return;
	}

	float maxFrameRotation = dT * maxTurnSpeed;
	float desiredTurn = transform->orientationTo(destinationPos);
	float frameRotation = std::copysign( glm::min(maxFrameRotation, abs(desiredTurn)),desiredTurn);
	transform->rotate(frameRotation); 
	
	float speed = speedFor(destinationPos);
	glm::vec3 impulse(0);
	if (abs(desiredTurn) < rad90) 
		impulse = transform->getRotationVec() * speed;
	physics.moveImpulse = impulse;

 	auto [avoidanceNeeded, risk] = findAvoidance();

	float maxOpposingFrameRotation = maxFrameRotation * 0.5f;


	if (abs(avoidanceNeeded) > 0) {

		//Special case: colliding bots side-by-side heading same way
		if (pRoboCollidee && pRoboCollidee->backingUp <= 0 &&
			glm::dot(transform->getRotationVec(),pRoboCollidee->transform->getRotationVec()) > 0.8f &&
			glm::distance(getPos(), pRoboCollidee->getPos()) < 1.0f) {
			backingUp = 0.55f; 
			return;
		}

		//does avoidance oppose proposed turn?
		if (signbit(avoidanceNeeded) != std::signbit(frameRotation)) {
			float frameAvoidance = std::copysign(glm::min(maxOpposingFrameRotation +  abs(frameRotation), abs(avoidanceNeeded)), avoidanceNeeded);
			transform->rotate(frameAvoidance);
		}
		else { //no? work with it
			float frameAvoidance = std::copysign(glm::min(maxFrameRotation, abs(avoidanceNeeded)), avoidanceNeeded);
			if (abs(frameAvoidance) > abs(frameRotation)) {
				float diff = std::copysign(abs(frameAvoidance) - abs(frameRotation), avoidanceNeeded);
				transform->rotate(diff);
			}
		}
	}

	physics.moveImpulse *= 1.0f - risk;
}

/** Rotate upper body to track a target, if any. */
void CRobot::trackTarget() {
	float targetAngle;
	switch (trackingState) {
	case trackEntity: 
		//targetAngle = glm::orientedAngle(glm::normalize(trackingEntity->worldPos - worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
		targetAngle = shortestAngle(transform->getUpperBodyRotation(), trackingEntity->getPos() - getPos());
		//setUpperRotation(targetAngle);
		turnUpperBodyTo(targetAngle);
		break;
	case trackPos:
		//targetAngle = glm::orientedAngle(glm::normalize(trackingPos - worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
		//setUpperRotation(targetAngle);
		targetAngle = shortestAngle(transform->getUpperBodyRotation(), trackingPos - getPos());
		turnUpperBodyTo(targetAngle);
		break;
	case trackEnding:
		float dist = fmod(rad360 + transform->getRotation() - transform->getUpperBodyRotation(), rad360);
		float turnStep = dT * upperTurnSpeed;
		if (turnStep < dist) {
			//put in range [-pi - pi] to give turn a direction, ie, clockwise/anti
			if (dist > M_PI)
				turnStep = -turnStep;
			transform->rotateUpperBody(turnStep);
		}
		else {
			transform->setUpperBodyRotation(transform->getRotation());
			trackingState = trackNone;
		}

	}

}

void CRobot::turnUpperBodyTo(float destAngle) {
	float frameturn = std::copysign(glm::min(upperTurnSpeed, abs(destAngle)), destAngle);
	transform->rotateUpperBody(frameturn);

}

void CRobot::updateTreadCycle() {
	if (!moving)
		return;

	float treadLoop = 0.3f;

	float velocityMod = 1.0f; //glm::length(physics.velocity) / 1.7f; //1.7f = usual max
	//scaling by velocity doesn't seem to have a visible effect. Shelf for now.

	treadCycle += dT * velocityMod;
	treadCycle = fmod(treadCycle, treadLoop); //loop after travelling treadLoop
	float f = 1.0f - treadCycle / treadLoop; // 0 - 1

	float treadGap = 0.1f;
	treadTranslate = f * treadGap;
	transform->setWalkTranslation(glm::vec3(-treadTranslate, 0, 0));
}


bool CRobot::canSeeEnemy() {
	if (game.player == nullptr)
		return false;
	return game.player->visible &&  !game.player->dead && inFov(game.player);
		//clearLineTo(world.player);
}


