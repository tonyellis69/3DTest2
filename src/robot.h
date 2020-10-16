#pragma once
#define _USE_MATH_DEFINES

#include "tigConst.h"

#include "gameMsg.h"

#include "goalActor.h"

#include "viewField.h"

enum TRobotState {robotSleep, robotChase};

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CGoalActor {
public:
	CRobot();
	void frameUpdate(float dT);
	bool update(float dT);
	void update2(float dT);
	void chooseRandomDestination();
	void draw();
	void leftClick();
	void leftClickPowerMode();

	void hitTarget();
	int getMissileDamage();

	bool canSee(CGameHexObj* target);


	void playerSight(bool inView);

	CViewFieldArc viewField;

	TRobotState state = robotSleep;
	bool travelling = false;
	CHex chaseHex;

private:
	int tigCall(int memberId);
	void onNotify(COnCursorNewHex& msg);

	void onNotify(CPlayerNewHex& msg);

	void onNotify(CActorMovedHex& msg);

	void deathRoutine();

	CHex getLastSeen();

	void updateViewField();

	void checkForPlayer();

	void moveReal();


	float robotMoveSpeed = 2.5f;


};


