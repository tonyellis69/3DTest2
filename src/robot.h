#pragma once
#define _USE_MATH_DEFINES

#include "tigConst.h"

#include "gameMsg.h"

//#include "actor.h"
#include "entity.h"

#include "viewField.h"

enum TRobotState {robotSleep, robotChase, robotWander, 
	robotMelee, robotShoot, robotHunt, robotLightSleep,
	robotEvasiveShoot, robotWander2, robotCharge, robotLookFor,
	robotLookAround, robotWander3, robotCharge3,
	robotMelee3
};

enum TLungeState {preLunge, lunging, returning};

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CEntity {
public:
	CRobot();
	void update(float dT);
	void setState(TRobotState newState, CEntity* entity = NULL);
	TRobotState getState();
	CHex getNextTravelHex(CHex& destination);
	void draw();




	std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB);

	void receiveDamage(CEntity& attacker, int damage);

	CViewFieldArc viewField;


	bool transitioningToHex = false;
	CHex destination = CHex(-1);
	CHex moveDest; ///<Adjacent hex we're moving to.

private:
//	int tigCall(int memberId);
	//void updateViewField();
	void approachDestHex();
	void rotateAlong(const float& angle);
	void melee();
	void melee3();
	bool hasLineOfSight(CEntity* target);
	bool hasLineOfSight(const glm::vec3& p);
	bool inFoV(CEntity* target);
	void fireMissile(CEntity* target);
	void strafe();
	void wander2();
	void wander3();
	void charge();
	void charge3();
	void lookFor();
	void lookAround();

	void onMovedHex();

	void approachDestination();

	float dT;

	TRobotState state = robotLightSleep;// robotWander3;// robotLightSleep;


	float robotMoveSpeed = 3.0f;// 2.5f;
	float robotRotateSpeed = 5.0f;// 5.0f;

	float robotLungeSpeed = 8;
	float robotMeleeRange = 1.8f;
	float robotLungeDistance = 1.0f;
	float meleeHitCooldown = 0;
	glm::vec3 lungeTarget = glm::vec3(0);
	glm::vec3 lungeDir = glm::vec3(0);
	bool lungeReturning = false;
	glm::vec3 lungeEndPos;

	float missileCooldown = 0.0f;
	CEntity* targetEntity = NULL;

	float evadeTimer = 0.0f;
	bool evadeShoot;
	bool hasFired = false;

	bool midRun = false;
	THexList pathTemp;

	glm::vec4 hostileColour = { 1,0,0,1 };
	glm::vec4 shootingColour = { 1,1,0,1 };


	bool tmpCharger = false;
	bool tmpPlayerInFov = false;

	float rotationSearched = 0;
	CHex targetLastHeading;
	THexDir targetLastDirection;

	
	glm::vec3 meleeLungeHome = { 0,0,0 };
	float lastDistance = 0;
	TLungeState lungeState;
	float meleeRange = 1.0f;

	glm::vec3 destinationWS = { 0,0,0 };
	float destSlowdownRange = 0.4f;
	float destSlowdownRate = 0.5f;
	bool reachedDestination = true;
	float lastDestinationDist = FLT_MAX;
	float speed = 0.0f;
};


