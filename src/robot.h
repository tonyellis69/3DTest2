#pragma once
#define _USE_MATH_DEFINES

#include "tigConst.h"

#include "gameMsg.h"

#include "goalActor.h"

#include "viewField.h"

enum TRobotState {robotSleep, robotChase, robotWander, 
	robotMelee};

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CGoalActor {
public:
	CRobot();
	void frameUpdate(float dT);
	void update2(float dT);
	void setState(TRobotState newState);
	TRobotState getState();
	CHex getNextTravelHex(CHex& destination);
	void draw();

	void hitTarget();
	int getMissileDamage();

	bool canSee(CGameHexObj* target);


	void playerSight(bool inView);

	std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB);

	void receiveDamage(CGameHexObj& attacker, int damage);

	CViewFieldArc viewField;


	bool travellingToHex = false;
	CHex destination = CHex(-1);

private:
	int tigCall(int memberId);
	void onNotify(COnCursorNewHex& msg);

	void onNotify(CPlayerNewHex& msg);



	void deathRoutine();

	CHex getLastSeen();

	void updateViewField();

	void checkForPlayer();

	void moveReal();

	void melee();


	TRobotState state = robotSleep;


	float robotMoveSpeed = 3.0f;// 2.5f;

	float robotLungeSpeed = 8;
	float robotMeleeRange = 1.8f;
	float robotLungeDistance = 1.0f;
	float meleeHitCooldown = 0;
	glm::vec3 lungeTarget = glm::vec3(0);
	glm::vec3 lungeDir = glm::vec3(0);
	bool lungeReturning = false;
	glm::vec3 lungeEndPos;
};


