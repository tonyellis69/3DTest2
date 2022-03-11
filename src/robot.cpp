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
void CRobot::setImpulse(glm::vec3& dest, float maxSpeed) {
	float proportionalSlowingDist = slowingDist * (maxSpeed / 1000);
	glm::vec3 targetOffset = dest - worldPos;
	float distance = glm::length(targetOffset);
	float rampedSpeed = maxSpeed * (distance / proportionalSlowingDist);
	float clippedSpeed = std::min(rampedSpeed, maxSpeed);
	physics.moveImpulse = (clippedSpeed / distance) * targetOffset;
	moving = true;
}

glm::vec3 CRobot::slowTo(glm::vec3& dest) {
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

	if (abs(turnDist) < 0.01f) //temp!!!!
		return true;

	float turnDir = (std::signbit(turnDist)) ? -1.0f : 1.0f;

	if (lastTurnDir != 0 && lastTurnDir != turnDir) { //we overshot
		float targetAngle = glm::orientedAngle(glm::normalize(p - worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
		setRotation(targetAngle);
		lastTurnDir = 0;
		return true;

	}


	float turnStep = turnDir * dT * 5.0f; //temp!
	//bot->rotation += turnStep;
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
glm::vec3 CRobot::findAvoidance() {
	float halfWidth = 0.55f;

	glm::vec3 travelDir = getRotationVec();
	lAvoidVec[0] = worldPos + glm::vec3(-travelDir.y, travelDir.x, travelDir.z) * halfWidth;
	rAvoidVec[0] = worldPos + glm::vec3(travelDir.y, -travelDir.x, travelDir.z) * halfWidth;

	lAvoidVec[1] = lAvoidVec[0] + travelDir * avoidanceDist;
	rAvoidVec[1] = rAvoidVec[0] + travelDir * avoidanceDist;

	//check for collision
	lObstacle = false; rObstacle = false;
	TIntersections solidHexes;
	auto hexes = getIntersectedHexes(lAvoidVec[0], lAvoidVec[1]);
	for (auto& hex : hexes) {
		if (game.map->getHexArray()->getHexCube(hex.first).content == solidHex) {
			solidHexes.push_back(hex);
		}
	}

	hexes = getIntersectedHexes(rAvoidVec[0], rAvoidVec[1]) ;
	for (auto& hex : hexes) {
		if (game.map->getHexArray()->getHexCube(hex.first).content == solidHex) {
			solidHexes.push_back(hex);
		}
	}

	if (solidHexes.empty())
		return glm::vec3(0);


	std::sort(solidHexes.begin(), solidHexes.end(), [&](const auto& a, const auto& b) {
		return glm::distance(worldPos, a.second) < glm::distance(worldPos, b.second); });




	//find which side of us collision point is on
	glm::vec3 pt = solidHexes.begin()->second;
	auto c = glm::cross(travelDir, pt - worldPos);
	glm::vec3 avoidanceVec;
	if (c.z > 0) {
		lObstacle = true;
		avoidanceVec = { travelDir.y, -travelDir.x, 0 };
	}
	else {
		rObstacle = true;

		avoidanceVec = { -travelDir.y, travelDir.x, 0 };
	}

	//find closest point along travel line:
	//project pt - worldPos along travel vec
	//find distance between = degree of avoidance needed?



	return avoidanceVec;
}


glm::vec3 CRobot::findAvoidance2() {
	float halfWidth = 0.55f;

	glm::vec3 travelDir = getRotationVec();
	lAvoidVec[0] = worldPos + glm::vec3(-travelDir.y, travelDir.x, travelDir.z) * halfWidth;
	rAvoidVec[0] = worldPos + glm::vec3(travelDir.y, -travelDir.x, travelDir.z) * halfWidth;

	lAvoidVec[1] = lAvoidVec[0] + travelDir * avoidanceDist;
	rAvoidVec[1] = rAvoidVec[0] + travelDir * avoidanceDist;

	//check for collision
	lObstacle = false; rObstacle = false;
	glm::vec3 aheadVec = travelDir * avoidanceDist;
	THexList solidHexes;

	CHex aheadHex = worldSpaceToHex(worldPos + aheadVec);
	THexList aheadHexes = getNeighbours(aheadHex);
	aheadHexes.push_back(aheadHex);

	for (auto& hex : aheadHexes) {
		if (game.map->getHexArray()->getHexCube(hex).content == solidHex) {
			solidHexes.push_back(hex);
		}
	}

	if (solidHexes.empty()) //do programatically!
		return glm::vec3(0);


	std::sort(solidHexes.begin(), solidHexes.end(), [&](const auto& a, const auto& b) {
		return glm::distance(worldPos, cubeToWorldSpace(a)) < glm::distance(worldPos, cubeToWorldSpace(b)); });


	//check for collision
	glm::vec3 pt = glm::vec3(0);
	float radius = 1.2f;
	for (auto& hex : solidHexes) {
		float dist = glm::distance(worldPos + aheadVec, cubeToWorldSpace(hex));
		if (dist < radius) {
			pt = cubeToWorldSpace(hex);
			break;
		}
	}

	if (pt == glm::vec3(0))
		return glm::vec3(0);

	//find which side of us collision point is on

	auto c = glm::cross(aheadVec, pt - worldPos);
	glm::vec3 avoidanceVec;
	if (c.z > 0) {
		lObstacle = true;
		avoidanceVec = { travelDir.y, -travelDir.x, 0 };
	}
	else {
		rObstacle = true;
		avoidanceVec = { -travelDir.y, travelDir.x, 0 };
	}

	glm::vec3 avoidForce = (worldPos + aheadVec) - pt;
	avoidForce = glm::normalize(avoidForce);

	return avoidForce;
}

void CRobot::headTo(glm::vec3 & pos) {
	//avoidanceDist = std::min(2.0f, glm::distance(pos, worldPos));

	glm::vec3 impulse = slowTo(pos);

	//check if we need to avoid anything
	glm::vec3 avoidVec = findAvoidance2();

	if (glm::length(avoidVec) > 0)
		int b = 0;

	impulse += avoidVec * 50.0f;


	bool facingDest = turnTo(worldPos + impulse);
	if (!facingDest)
		return ;

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
	float f = treadCycle / treadLoop; // 0 - 1

	float treadGap = 0.1f;
	treadTranslate = f * treadGap;
}


bool CRobot::canSeePlayer() {

	return game.player->visible &&  !game.player->dead && inFov(game.player);
		//clearLineTo(world.player);
}


