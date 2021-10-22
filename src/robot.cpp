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
const float rad60 = M_PI / 3;
const float rad45 = M_PI / 4;
const float rad120 = rad360 / 3;

CRobot::CRobot() {
	//lineModel = hexRendr2.getLineModel("robot");
	setBoundingRadius();
	isRobot = true;
	physics.invMass = 1.0f / 80.0f; //temp!
}


void CRobot::update(float dT) {
	this->dT = dT;

	if (currentState) {
		auto newState = currentState->update(dT);
		if (newState)
			currentState = newState;
	}

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
bool CRobot::hasLineOfSight(CEntity* target) {
	return hasLineOfSight(target->worldPos);
}

bool CRobot::hasLineOfSight(const glm::vec3& p) {
	TIntersections intersectedHexes = getIntersectedHexes(worldPos, p);
	for (auto& hex : intersectedHexes) {
		if (game.map->getHexArray()->getHexCube(hex.first).content != emptyHex)
			return false;
		//TO DO: can expand this to check for other robots blocking
	}

	return true;
}


bool CRobot::inFov(CEntity* target) {
	if (glm::distance(worldPos, target->worldPos) <= 12 && 
		abs(orientationTo(target->worldPos)) < rad120 /*rad45*/) { //rad60
		if (hasLineOfSight(target->worldPos))
			return true;
	}

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
		rotation = glm::orientedAngle(glm::normalize(p - worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
		lastTurnDir = 0;
		return true;
	}

	float turnStep = turnDir* dT * 5.0f; //temp!
	rotation += turnStep;

	lastTurnDir = turnDir;
	return false;
}



bool CRobot::canSeePlayer() {

	return game.player->visible &&  !game.player->dead && inFov(game.player);
		//hasLineOfSight(world.player);
}


