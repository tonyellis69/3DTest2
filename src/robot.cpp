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

const float rad360 = M_PI * 2;
const float rad90 = M_PI / 2;
const float rad80 = 1.39626;
const float rad70 = 1.22173;
const float rad60 = M_PI / 3;
const float rad50 = 0.872665;
const float rad45 = M_PI / 4;
const float rad40 = 0.698132;
const float rad120 = rad360 / 3;

CRobot::CRobot() {

	isRobot = true;
	physics.invMass = 1.0f / 80.0f; //temp!
}

void CRobot::setModel(TModelData& model) {
	lineModel.model = model;
	upperBody = lineModel.getNode("body");
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
	buildWorldMatrix();
}


/** Switch to the given state, performing the necessary initialisation. */
void CRobot::setState(TRobotState newState, CEntity* entity)
{
	switch (newState) {
	case robotWander3:
		currentState = std::make_shared<CRoboWander>(this);
		lineModel.setColourR(glm::vec4(0, 1, 0, 1)); //move inside!
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
	CEntity::draw();
}


/** Check if the given segment intersects this robot. */
std::tuple<bool, glm::vec3> CRobot::collisionCheck(glm::vec3& segA, glm::vec3& segB) {
	if (lineModel.BBcollision(segA, segB))
		return { true, glm::vec3() };
	return { false, glm::vec3()};
}

void CRobot::buildWorldMatrix() {
	lineModel.model.matrix = glm::translate(glm::mat4(1), worldPos);
	lineModel.model.matrix = glm::rotate(lineModel.model.matrix, rotation, glm::vec3(0, 0, -1));
	//NB: we use a CW system for angles

	upperBody->matrix = glm::translate(glm::mat4(1), worldPos);
	upperBody->matrix = glm::rotate(upperBody->matrix, upperBodyRotation, glm::vec3(0, 0, -1));

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

	auto missile = (CMissile*) spawn::missile("missile", worldPos, targetAngle);
	missile->setOwner(this);
	missile->setSpeed(7.0f);

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


bool CRobot::canSeePlayer() {

	return game.player->visible &&  !game.player->dead && inFov(game.player);
		//clearLineTo(world.player);
}


