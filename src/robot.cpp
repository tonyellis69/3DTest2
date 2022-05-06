#include "robot.h"

#include <cmath>

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

const float rad360 = float(M_PI) * 2.0f;
const float rad90 = float(M_PI) / 2;
const float rad80 = 1.39626;
const float rad70 = 1.22173;
const float rad60 = float(M_PI) / 3;
const float rad50 = 0.872665;
const float rad45 = float(M_PI) / 4;
const float rad40 = 0.698132;
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
	setBoundingRadius();
}


void CRobot::update(float dT) {
	this->dT = dT;

	if (currentState) {
		auto newState = currentState->update(dT);
		if (newState)
			currentState = newState;
	}

	if (trackingState)
		trackTarget();

	updateTreadCycle();

	if (moving)
		amIStuck();

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

	//if (abs(turnDist) < 0.01f) { //temp!!!!
	//	return true;
	//}

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
glm::vec3 CRobot::findAvoidance2() {
	float robotRadius = 0.7f;

	glm::vec3 travelDir = getRotationVec();

	//check for collision
	float destinationDist = glm::distance(worldPos, *getDestination());
	float avoidanceDist = std::min(maxAvoidanceDist, destinationDist) ;
	float aheadSegStartDist = 0.15f;// robotRadius * 0.75f;//  1.0f;


	glm::vec3 aheadVec = travelDir * avoidanceDist;
	glm::vec3 aheadSegEnd = worldPos + aheadVec;
	glm::vec3 aheadSegBegin = worldPos +travelDir * aheadSegStartDist;
	glm::vec3 aheadSegCentre = aheadSegBegin + (aheadSegEnd - aheadSegBegin) * 0.5f;
	tmpAheadVecBegin = aheadSegBegin;



	tmpCollisionPt = glm::vec3(0);
	tmpCollisionSegPt = aheadSegBegin;
	tmpAheadVecEnd = aheadSegEnd;

	if (destinationDist < aheadSegStartDist) {//destination closer than aheadCheck? Skip avoidance 
		return glm::vec3(0);
	}


	CHex midwayHex = worldSpaceToHex(aheadSegCentre);
	THexList aheadHexes = getNeighbours(midwayHex,2);

	//find nearest obstacle
	std::vector<TObstacle> obstacles = findNearObstacles(aheadSegCentre);


	if (obstacles.empty()) 
		return glm::vec3(0);

	////nearest obstacles first
	std::sort(obstacles.begin(), obstacles.end(), [&](const auto& a, const auto& b) {
		return glm::distance(worldPos, a.pos) < glm::distance(worldPos, b.pos); });


	////check for collision
	glm::vec3 obstacleCentre = glm::vec3(0);
	float radius;
	float distance = 0;
	glm::vec3 collision;
	for (auto& obstacle : obstacles) {
		//find nearest point on ahead vector to hex origin
		glm::vec3 pos = obstacle.pos;
		collision = closestPointSegment(aheadSegBegin, aheadSegEnd, pos);
		if (collision == aheadSegBegin)
			continue; //assume obstacle behind us
		distance = glm::distance(collision, pos);
		radius = obstacle.radius + robotRadius;
		if (distance < radius ) { //!!!
			obstacleCentre = pos;
			tmpCollisionSegPt = collision;
			break;
		}
	}

	if (obstacleCentre == glm::vec3(0))
		return glm::vec3(0);

	tmpCollisionPt = obstacleCentre;


	//Find vector from hex centre to collision point
	glm::vec3 collisionVec = collision - obstacleCentre;
	glm::vec3 collisionVecN = glm::normalize(collisionVec);
	glm::vec3 avoidVec = collisionVecN * (radius - distance);

	float collisionDist = glm::distance(worldPos,collision);

	return avoidVec / collisionDist;
}


void CRobot::headTo(glm::vec3 & pos) {
	glm::vec3 impulse = arriveAt(pos);

	glm::vec3 avoidVec = glm::vec3(0);

	//if we're not facing destination, turn before
	//checking for obstacles
	bool facingDest = turnTo(pos);
	if (!facingDest)
		return;

	avoidVec = findAvoidance2();

	float impulseStrength = glm::length(impulse);
	impulse += avoidVec * impulseStrength; 

	turnTo(worldPos + impulse); //have to turn again in case we're avoiding (confirm)

	physics.moveImpulse = impulse;
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


