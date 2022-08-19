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

const float rad360 = float(M_PI) * 2.0f;
const float rad90 = float(M_PI) / 2;
const float rad80 = 1.39626;
const float rad70 = 1.22173;
const float rad60 = float(M_PI) / 3;
const float rad50 = 0.872665;
const float rad45 = float(M_PI) / 4;
const float rad40 = 0.698132;
const float rad30 = float(M_PI) / 6;
const float rad120 = rad360 / 3;

CRobot::CRobot() {

	isRobot = true;
	physics.invMass = 1.0f / 80.0f; //temp!
}


void CRobot::setModel(CModel& model) {
	this->model = model;
	upperBody = this->model.getMesh("robody");
	base = this->model.getMesh("robase");
	treads = this->model.getMesh("treads");
	upperBodyMask = this->model.getMesh("robody_mask");
	robaseMask = this->model.getMesh("robase_mask");
	setBoundingRadius();
}


void CRobot::update(float dT) {
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

	if (moving) 
		amIStuck(); //FIX: crudely geared to destination only

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

void CRobot::setRotation(float angle) {
	if (upperBodyLocked) {
		float diff = upperBodyRotation - rotation;
		upperBodyRotation = angle + diff;
		upperBodyRotation = fmod(upperBodyRotation + rad360, rad360);
	}
	rotation = angle;
}

void CRobot::rotate(float angle) {
	rotation += angle;
	rotation = fmod(rotation + rad360, rad360);
	if (upperBodyLocked) {
		upperBodyRotation += angle;
		upperBodyRotation = fmod(upperBodyRotation + rad360, rad360);
	}
}

void CRobot::setUpperRotation(float angle) {
	upperBodyRotation = angle;
}

void CRobot::rotateUpper(float angle) {
	upperBodyRotation += angle;
	upperBodyRotation = fmod(upperBodyRotation + rad360, rad360);
}

void CRobot::initDrawFn() {
	drawFn = std::make_shared<CMultiDraw>(this);
	auto fn = (CMultiDraw*)drawFn.get();
	fn->lowerMeshes.push_back(model.getMesh("robase"));
	fn->lowerMeshes.push_back(model.getMesh("treads"));
	fn->upperMeshes.push_back(model.getMesh("robody"));
	fn->upperMask = model.getMesh("robody_mask");
	fn->lowerMasks.push_back(model.getMesh("robase_mask"));


	//drawFn = std::make_shared<CMultiDraw>(this);
	//auto fn = (CMultiDraw*)drawFn.get();
	//fn->lowerMeshes.push_back(model.getMesh("footL"));
	//fn->lowerMeshes.push_back(model.getMesh("footR"));
	//fn->upperMeshes.push_back(model.getMesh("body"));
	//fn->upperMask = model.getMesh("body_mask");
}

void CRobot::draw() {
	//if (!visibleToPlayer)
	//	return;
	//for (auto hex : viewField.visibleHexes)
	//	hexRendr2.highlightHex(hex);
	//CEntity::draw();
	hexRendr2.drawLineModel(*upperBody);
	hexRendr2.drawLineModel(*base);
	hexRendr2.drawLineModel(*treads);
}


/** Check if the given segment intersects this robot. */
std::tuple<bool, glm::vec3> CRobot::collisionCheck(glm::vec3& segA, glm::vec3& segB) {
	if (model.BBcollision(segA, segB))
		return { true, glm::vec3() };
	return { false, glm::vec3()};
}

void CRobot::buildWorldMatrix() {
	glm::mat4 worldM = glm::translate(glm::mat4(1), worldPos);

	upperBody->matrix = worldM;
	upperBody->matrix = glm::rotate(worldM, upperBodyRotation, glm::vec3(0, 0, -1));

	upperBodyMask->matrix = upperBody->matrix;

	worldM = glm::rotate(worldM, rotation, glm::vec3(0, 0, -1));

	base->matrix = worldM;

	model.tmpMatrix = upperBody->matrix;
	//FIXME! Temp kludge to ensure collision check works
	//maybe solve by giving every model a collision subModel to check against.

	treads->matrix = glm::translate(worldM, glm::vec3(-treadTranslate, 0, 0));
	robaseMask->matrix = base->matrix;
}

void CRobot::startTracking(CEntity* target) {
	trackingState = trackEntity;
	trackingEntity = target;
	upperBodyLocked = false;
}

void CRobot::startTracking(glm::vec3& pos) {
	trackingState = trackPos;
	trackingPos = pos;
	upperBodyLocked = false;
}

void CRobot::stopTracking() {
	trackingState = trackEnding;
	upperBodyLocked = true;
}



void CRobot::receiveDamage(CEntity& attacker, int damage) {
	hp--;
	if (hp == 0) {
		game.killEntity(*this);
		spawn::explosion("explosion", worldPos, 1.5f);
	}
}

/** Give robot a push toward this destination, which will scale down
	with proximity to avoid overshooting. */
glm::vec3 CRobot::arriveAt(glm::vec3& dest) {
	float proportionalSlowingDist = slowingDist * (chosenSpeed / 1000);
	glm::vec3 targetOffset = dest - worldPos;
	float distance = glm::length(targetOffset);
	float rampedSpeed = chosenSpeed * (distance / proportionalSlowingDist);
	float clippedSpeed = std::min(rampedSpeed, chosenSpeed);
	moving = true;
	return (clippedSpeed / distance) * targetOffset;
}

float CRobot::speedFor(glm::vec3& dest) {
	float proportionalSlowingDist = slowingDist * (chosenSpeed / 1000);
	glm::vec3 targetOffset = dest - worldPos;
	float distance = glm::length(targetOffset);
	float rampedSpeed = chosenSpeed * (distance / proportionalSlowingDist);
	float clippedSpeed = std::min(rampedSpeed, chosenSpeed);
	moving = true;
	return clippedSpeed;
}


/** Return true if we can draw a line to the target without hitting anything. */
bool CRobot::clearLineTo(CEntity* target) {
	return clearLineTo(target->worldPos);
}

bool CRobot::clearLineTo(const glm::vec3& p) {
	TIntersections intersectedHexes = getIntersectedHexes(worldPos, p);
	for (auto& hex : intersectedHexes) {
		if (game.map->getHexArray()->getHexCube(hex.first).content != emptyHex)
			return false;
		//TO DO: can expand this to check for other robots blocking
	}

	return true;
}


bool CRobot::inFov(CEntity* target) {
	glm::vec3 targetDir = target->worldPos - worldPos;
	targetDir = glm::normalize(targetDir);

	//find upper body rotation as a vector
	glm::vec3 rotVec = { cos(upperBodyRotation), -sin(upperBodyRotation),0 };
	if ((glm::dot(rotVec, targetDir)) < cos(rad40))
		return false;

	if ( glm::distance(worldPos, target->worldPos) <= 12 && 
		clearLineTo(target->worldPos) )
			return true;
	
	return false;
}

std::vector<TObstacle> CRobot::findNearObstacles(glm::vec3& centre) {
	CHex midwayHex = worldSpaceToHex(centre);
	THexList aheadHexes = getNeighbours(midwayHex, 2);

	std::vector<TObstacle> obstacles;

	for (auto& hex : aheadHexes) {
		if (game.map->getHexArray()->getHexCube(hex).content == solidHex) {
			 obstacles.push_back({ cubeToWorldSpace(hex), hexSize });
		}
		else {
			CEntities entities = game.map->getEntitiesAt(hex);
			for (auto& entity : entities) {
				if (entity->isRobot && entity != this)
					obstacles.push_back({ entity->worldPos,1.0f /* entity->getRadius()*/ });
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
		float newDist = glm::distance(worldPos, *getDestination());
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
	glm::vec3 targetVec = target->worldPos - worldPos;
	float targetAngle = glm::orientedAngle(glm::normalize(targetVec), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

	auto missile = (CMissile*) spawn::missile("missile", worldPos, targetAngle).get();
	missile->setOwner(this);
	missile->setSpeed(15);// 7.0f);

	snd::play("shoot");
}


void CRobot::onMovedHex()
{
}

/** Continue turning toward p, if not facing it. */
bool CRobot::turnTo(glm::vec3& p) {
	float turnDist = orientationTo(p);

	float turnDir = (std::signbit(turnDist)) ? -1.0f : 1.0f;

	if (lastTurnDir != 0 && lastTurnDir != turnDir) { //we overshot
		float targetAngle = glm::orientedAngle(glm::normalize(p - worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
		setRotation(targetAngle);
		lastTurnDir = 0;
		return true;

	}

	float turnStep = turnDir * dT * 5.0f; //temp!
	rotate(turnStep);
	lastTurnDir = turnDir;
	return false;
}



void CRobot::stopMoving() {
	moving = false;
	physics.moveImpulse = { 0,0,0 };
}

glm::vec3* CRobot::getDestination() {
	return currentState->getDestination();
}

/** Return the necessary vector to avoid obstacles ahead. */
std::tuple<float, float> CRobot::findAvoidance2() {
	float robotRadius = 0.7f;

	glm::vec3 travelDir = getRotationVec();

	//check for collision
	float destinationDist = glm::distance(worldPos, *getDestination());
	float farCheckDist = std::min(maxAvoidanceDist, destinationDist) ;
	float nearCheckDist = robotRadius * 0.75f; // 0.75f; // +0.1f;// 0.15f;// robotRadius * 0.75f;//  1.0f;

	if (destinationDist < nearCheckDist) //destination closer than check start? Skip avoidance 
		return { 0,0 };

	glm::vec3 destinationVec = glm::normalize(*getDestination() - worldPos);

	//bail if facing away from destination
	float angleToDest = glm::dot((travelDir), destinationVec);
	if (angleToDest < 0)
		return { 0,0 };
		//angleToDest = 0.1f; //CHECK: may not need this anymore

	//farCheckDist *= angleToDest; //reduce if we're not pointing where going
	if (farCheckDist < nearCheckDist)
		return { 0,0 };

	//find vec between dest and current heading
	//check that  for obstacles
	glm::vec3 exploreVec = travelDir;// glm::normalize(destinationVec + travelDir);


	glm::vec3 aheadSegEnd = worldPos + (exploreVec * farCheckDist);
	glm::vec3 aheadSegBegin = worldPos + (exploreVec * nearCheckDist);
	glm::vec3 aheadSegCentre = aheadSegBegin + (aheadSegEnd - aheadSegBegin) * 0.5f;

	//for graphic
	tmpAheadVecBegin = aheadSegBegin; 
	tmpCollisionPt = glm::vec3(0);
	tmpCollisionSegPt = aheadSegBegin;
	tmpAheadVecEnd = aheadSegEnd;

	//find nearest obstacle
	std::vector<TObstacle> obstacles = findNearObstacles(aheadSegCentre);
	if (obstacles.empty()) 
		return { 0,0 };

	//nearest obstacles first
	std::sort(obstacles.begin(), obstacles.end(), [&](const auto& a, const auto& b) {
		return glm::distance(worldPos, a.pos) < glm::distance(worldPos, b.pos); });


	//check for collision
	glm::vec3 obstacleCentre = glm::vec3(0);
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
		safeDist = obstacle.radius + robotRadius;
		if (distanceToSegment < safeDist ) { 
			obstacleCentre = obstacle.pos;
			tmpCollisionSegPt = aheadSegNearestPt;
			break;
		}
	}

	if (obstacleCentre == glm::vec3(0))
		return { 0,0 };

	tmpCollisionPt = obstacleCentre;


	//need to find desired rotation to move aheadSeg out of the bounding circle of the obstacle
	//(+ radius of robot). Has to be in direction toward 'free' space, even if that means passing
	//aheadSeg through bulk of the obstacle.

	//Find vector from obstacle centre to collision point
 	glm::vec3 collisionVec = aheadSegNearestPt - obstacleCentre;
	glm::vec3 collisionVecN = glm::normalize(collisionVec);
	glm::vec3 avoidVec = collisionVecN * (safeDist - distanceToSegment); //off centre = smaller push

	//find what side of destination vector obstacle is
	//make a vector, freeSpaceDir, pointing the other way
	glm::vec3 freeSpaceDir;
	glm::vec3 perp = turnRight(destinationVec);
	if (glm::dot(obstacleCentre - worldPos, perp) > 0)
		//obstacle is on the ‘right’ side of the line.
		freeSpaceDir = turnLeft(travelDir);
	else
		freeSpaceDir = turnRight(travelDir);

	
	//if freespacedir and collisonVec point same way, we're on the short side of the obstacle
	if (glm::dot(freeSpaceDir, collisionVec) > 0) {
		avoidVec = collisionVecN * (safeDist - distanceToSegment);
	}
	else { //on far side
		avoidVec = -collisionVecN * (safeDist + distanceToSegment);
	}

	//find rotation required
	glm::vec3 safePt = glm::normalize(aheadSegNearestPt - worldPos + avoidVec);
	float avoidAngle = glm::orientedAngle(safePt, exploreVec, glm::vec3(0, 0, 1));

	//near enough for caution?

	float risk = 0;

	float collisionDist = glm::distance(obstacleCentre, worldPos);
	if (collisionDist - obstacleRadius < 1.0f && glm::dot(travelDir, glm::normalize(obstacleCentre - worldPos)) < rad60)
			risk = 1.0f - (collisionDist - obstacleRadius);


	return { avoidAngle, risk };

}


void CRobot::headTo(glm::vec3 & destinationPos) {
	glm::vec3 impulse = arriveAt(destinationPos);

	float desiredTurn = orientationTo(destinationPos);
	float maxTurn = dT * 4.0f; //FIX: use constant

	float frameRotation = std::copysign( glm::min(maxTurn, abs(desiredTurn)),desiredTurn);

	float speed = speedFor(destinationPos); 

	rotate(frameRotation);

	//find avoidance needed
 	auto [avoidRot, proximity] = findAvoidance2();

	float avoidanceRotation = 0;

	if (abs(avoidRot) > 0) {
		//does avoidance oppose proposed turn?
		if (signbit(avoidRot) != std::signbit(frameRotation)) {
			avoidRot += frameRotation;
			avoidanceRotation = std::copysign(glm::min(maxTurn, abs(avoidRot)), avoidRot);
			rotate(avoidanceRotation + -frameRotation);
		}
		else {
			avoidanceRotation = std::copysign(glm::min(maxTurn, abs(avoidRot)), avoidRot);
			if (abs(avoidanceRotation) > abs(frameRotation)) {
				float diff = std::copysign(abs(avoidanceRotation) - abs(frameRotation), avoidRot);
				rotate(diff);
			}
		}
	}



	if (abs(desiredTurn) > rad90) {
		physics.moveImpulse = glm::vec3(0);
		return;
	}
	//TO DO: investigate not moving until <60
	//investigate moving segA closer
	//TO DO: combine with slowing

	glm::vec3 newImpulse = getRotationVec();
	physics.moveImpulse = newImpulse * speed * (1.0f - proximity);


	if (proximity > 0)
		diagnostic += " Slow! x" + std::to_string(1 - proximity);


	//TO DO: need a new way to enforce slow travel when close to obstacle:
	// 
	//if (glm::length(avoidRot) > 0 && glm::length(avoidRot) < 0.9) {
	//	diagnostic += " Slow! ";
	//	physics.moveImpulse *= 0.25f;
	//}


}

/** Rotate upper body to track a target, if any. */
void CRobot::trackTarget() {
	float targetAngle;
	switch (trackingState) {
	case trackEntity: 
		targetAngle = glm::orientedAngle(glm::normalize(trackingEntity->worldPos - worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
		setUpperRotation(targetAngle);
		break;
	case trackPos:
		targetAngle = glm::orientedAngle(glm::normalize(trackingPos - worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
		setUpperRotation(targetAngle);
		break;
	case trackEnding:
		float dist = fmod(rad360 + rotation - upperBodyRotation, rad360);
		float turnStep = dT * upperTurnSpeed;
		if (turnStep < dist) {
			//put in range [-pi - pi] to give turn a direction, ie, clockwise/anti
			if (dist > M_PI)
				turnStep = -turnStep;
			rotateUpper(turnStep);
		}
		else {
			setUpperRotation(rotation);
			trackingState = trackNone;
		}

	}

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
}


bool CRobot::canSeePlayer() {

	return game.player->visible &&  !game.player->dead && inFov(game.player);
		//clearLineTo(world.player);
}


