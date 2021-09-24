#include "roboState.h"

#include <cmath>
#include <glm/gtx/vector_angle.hpp>

#include "robot.h"

#include "utils/random.h"

#include "utils/log.h"

const float rad360 = M_PI * 2;
const float rad80 = 1.39626;
const float rad90 = M_PI / 2;
const float rad60 = M_PI / 3;
const float rad45 = M_PI / 4;
const float rad120 = rad360 / 3;

CRoboWander::CRoboWander(CRobot* bot) : CRoboState(bot) {
	speed = 1000.0f;
	THexList ring;
	CHex randHex;
	int dist = 5;
	do {
		int giveUp = 0;
		ring = findRing(dist, bot->hexPosition);
		do {
			randHex = ring[rnd::dice(ring.size()) - 1];
			//can we los randHex?
			glm::vec3 hexWS = cubeToWorldSpace(randHex);
			if (bot->hasLineOfSight(hexWS)) {
				destination = hexWS;
				turnDestination = glm::orientedAngle(glm::normalize(hexWS - bot->worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
				lastDestinationDist = FLT_MAX;
				return;
			}
			giveUp++;

		} while (giveUp < 10); //temp!!!!!!!!!!!
		dist--;

	} while (dist > 0);

}

std::shared_ptr<CRoboState> CRoboWander::update(float dT) {
	this->dT = dT;

	bool facingDest = turnTo(destination);
	if (!facingDest)
		return nullptr;

	//proceed to actual movement
	glm::vec3 moveVec = glm::normalize(destination - bot->worldPos);
	bot->physics.moveImpulse = moveVec * speed;

	float dist = glm::distance(bot->worldPos, destination);

	if (dist < 0.05f) {
		bot->physics.velocity = { 0, 0, 0 };
		return std::make_shared<CGlanceAround>(bot); //temp!
	}

	if (dist < destSlowdownRange) { //time to slow down

		if (dist > lastDestinationDist) { //overshot!
			bot->physics.velocity = { 0, 0, 0 };
			return std::make_shared<CGlanceAround>(bot); //temp!
		}

		float p = dist / destSlowdownRange;
		bot->physics.moveImpulse *= p;
	}

	lastDestinationDist = dist;

	return nullptr;
}


/** Continue turning toward p, if not facing it. */
bool CRoboWander::turnTo(glm::vec3& p) {
	float turnDist = bot->orientationTo(p);

	if (abs(turnDist) < 0.01f) //temp!!!!
		return true;

	float turnDir = (std::signbit(turnDist)) ? -1.0f : 1.0f;

	if (lastTurnDir != 0 && lastTurnDir != turnDir) { //we overshot
		bot->rotation = glm::orientedAngle(glm::normalize(p - bot->worldPos), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
		lastTurnDir = 0;
		return true;

	}


	float turnStep = turnDir * dT * 5.0f; //temp!
	bot->rotation += turnStep;

	lastTurnDir = turnDir;
	return false;
}


CGlanceAround::CGlanceAround(CRobot* bot) : CRoboState(bot) {
	float leftLimit = fmod(rad360 + bot->rotation - rad90, rad360);
	float rightLimit = fmod(rad360+ bot->rotation + rad90, rad360);
	cumulativeRotation = 0;
	totalRotation = 0;

	glances = { leftLimit,bot->rotation,rightLimit,bot->rotation };
			//,leftLimit,bot->rotation,rightLimit,bot->rotation };
}

std::shared_ptr<CRoboState> CGlanceAround::update(float dT) {
	if (glances.empty()) {
		pause += dT;
		if (pause < 0.5f)
			return nullptr;

		return std::make_shared<CRoboWander>(bot);
	}

	float dest = glances.front();

	//find shortest angle between dest angle and our direction
	float turnDist; 
	if (totalRotation == 0) {
		turnDist = fmod(rad360 + dest - bot->rotation, rad360);
		//put in range [-pi - pi] to give angle a direction, ie, clockwise/anti
		if (turnDist > M_PI)
			turnDist = -(rad360 - turnDist);

		totalRotation = abs(turnDist);
		turnDir = (std::signbit(turnDist)) ? -1.0f : 1.0f;
	}


	float frameTurn = dT * glanceSpeed * turnDir;
	cumulativeRotation += abs(frameTurn);
	if (cumulativeRotation > totalRotation) { //overshot
		bot->rotation = dest;
		totalRotation = 0;
		cumulativeRotation = 0;
		glances.erase(glances.begin());
	}
	else {
		bot->rotation += frameTurn;
		//bot->rotation = fmod(rad360 + bot->rotation, rad360); //needed??
	}

	return nullptr;
}
