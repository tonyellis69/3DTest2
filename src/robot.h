#pragma once
#define _USE_MATH_DEFINES

#include "tigConst.h"

#include "gameMsg.h"

//#include "actor.h"
#include "gamehextObj.h"

#include "viewField.h"

enum TRobotState {robotSleep, robotChase, robotWander, 
	robotMelee, robotShoot, robotHunt, robotLightSleep,
	robotEvasiveShoot};

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CGameHexObj  {
public:
	CRobot();
	void update(float dT);
	void setState(TRobotState newState);
	TRobotState getState();
	CHex getNextTravelHex(CHex& destination);
	void draw();

	bool canSee(CGameHexObj* target);


	void playerSight(bool inView);

	std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB);

	void receiveDamage(CGameHexObj& attacker, int damage);

	CViewFieldArc viewField;


	bool tranistioningToHex = false;
	CHex destination = CHex(-1);

private:
	int tigCall(int memberId);
	void onNotify(COnCursorNewHex& msg);
	void updateViewField();
	void approachDestHex();
	void rotateAlong(const float& angle);
	void melee();
	bool hasLineOfSight(CGameHexObj* target);
	void fireMissile(CGameHexObj* target);
	void strafe();

	float dT;

	TRobotState state = robotLightSleep;


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
	CGameHexObj* targetEntity;

	float evadeTimer = 0.0f;
	bool evadeShoot;
	bool hasFired = false;

	bool midRun = false;
	THexList pathTemp;

	glm::vec4 hostileColour = { 1,0,0,1 };
	glm::vec4 shootingColour = { 1,1,0,1 };

};


