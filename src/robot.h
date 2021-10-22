#pragma once
#define _USE_MATH_DEFINES

#include "entity.h"
#include "roboState.h"

enum TRobotState { robotLightSleep,	robotWander3, robotCharge3,
	robotMelee3, robotCloseAndShoot, robotDoNothing
};



enum TTracking {trackNone, trackTarget, trackDestination};

enum TLungeState { preLunge, lunging, returning };

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CEntity {
public:
	CRobot();
	void update(float dT);
	void setState(TRobotState newState, CEntity* entity = NULL);
	void draw();
	std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB);

	void receiveDamage(CEntity& attacker, int damage);
	void setImpulse(glm::vec3& dest, float maxSpeed);
	bool hasLineOfSight(const glm::vec3& p);
	bool canSeePlayer();
	bool hasLineOfSight(CEntity* target);
	void fireMissile(CEntity* target);

private:
	bool inFov(CEntity* target);
	void onMovedHex();
	bool turnTo(glm::vec3& p);

	float dT;
	int hp = 3; 

	std::shared_ptr<CRoboState> currentState = nullptr; ///<The robot's current behaviour.

	float slowingDist = 0.6f;
	float lastTurnDir = 0;

};


