#pragma once
#define _USE_MATH_DEFINES


#include "entity.h"



enum TRobotState { robotLightSleep,	robotWander3, robotCharge3,
	robotMelee3, robotCloseAndShoot
};

enum TLungeState {preLunge, lunging, returning};

enum TTracking {trackNone, trackTarget, trackDestination};

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CEntity {
public:
	CRobot();
	void update(float dT);
	void setState(TRobotState newState, CEntity* entity = NULL);
	TRobotState getState();
	void draw();

	std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB);

	void receiveDamage(CEntity& attacker, int damage);


private:
	void melee3();
	bool hasLineOfSight(CEntity* target);
	bool hasLineOfSight(const glm::vec3& p);
	bool inFov(CEntity* target);
	void fireMissile(CEntity* target);

	void wander3();
	void charge3();
	void closeAndShoot();

	void onMovedHex();

	void approachDestination();
	void track();

	bool canSeePlayer();

	float dT;

	TRobotState state = robotWander3;// robotLightSleep;// robotWander3;// 

	float meleeHitCooldown = 0;
	float missileCooldown = 0.0f;

	CEntity* targetEntity = NULL;

	glm::vec4 hostileColour = { 1,0,0,1 };
	glm::vec4 shootingColour = { 1,1,0,1 };


	glm::vec3 meleeLungeHome = { 0,0,0 };
	float lastDistance = 0;
	TLungeState lungeState;
	float meleeRange = 1.0f;

	glm::vec3 destination = { 0,0,0 };


	float destSlowdownRange = 0.4f;
	float destSlowdownRate = 0.5f;
	bool reachedDestination = true;
	float lastDestinationDist = FLT_MAX;

	float speed = 0.0f;

	bool stoppedToShoot = false;
	float idealShootRange = 3;
	float escapeRange = idealShootRange + 1;

	TTracking tracking = trackNone;

};


