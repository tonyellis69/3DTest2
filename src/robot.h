#pragma once
#define _USE_MATH_DEFINES

#include "entity.h"
#include "roboState.h"

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
	bool hasLineOfSight(const glm::vec3& p);

	//wander stuff - try to encapsulate
	glm::vec3 destination = { 0,0,0 };
	float speed = 0.0f;
	bool reachedDestination = true;
	float destSlowdownRange = 0.4f;
	float destSlowdownRate = 0.5f;
	float lastDestinationDist = FLT_MAX;

	float turnDestination = 0; ///<Bearing we want to turn to


private:
	void melee3();
	bool hasLineOfSight(CEntity* target);

	bool inFov(CEntity* target);
	void fireMissile(CEntity* target);

	void wander3();
	void charge3();
	void closeAndShoot();

	void onMovedHex();

	void approachDestination();
	bool approachTurn();
	bool turnTo(glm::vec3& p);
	void track();

	bool canSeePlayer();

	float dT;

	TRobotState state = robotWander3;// robotLightSleep;// robotLightSleep;// robotWander3;// 

	int hp = 3; 

	std::shared_ptr<CRoboState> currentState = nullptr; ///<The robot's current behaviour.

	float meleeHitCooldown = 0;
	float missileCooldown = 0.0f;

	CEntity* targetEntity = NULL;

	glm::vec4 hostileColour = { 1,0,0,1 };
	glm::vec4 shootingColour = { 1,1,0,1 };


	float wakeRange = 8;
	glm::vec3 meleeLungeHome = { 0,0,0 };
	float lastDistance = 0;
	TLungeState lungeState;
	float meleeRange = 1.0f;


	bool finishedTurn = true;
	float glanceA;
	float glanceB;




	
	bool glancing = false;
	float glancePeriod;
	float oldRotation;

	float lastTurnDir = 0;


	bool stoppedToShoot = false;
	float idealShootRange = 3;
	float escapeRange = idealShootRange + 1;

	TTracking tracking = trackNone;

};


