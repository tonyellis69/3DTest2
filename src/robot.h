#pragma once
#define _USE_MATH_DEFINES

#include "tigConst.h"

#include "gameMsg.h"

#include "goalActor.h"

#include "viewField.h"

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CGoalActor {
public:
	CRobot();
	void chooseTurnAction();
	void frameUpdate(float dT);
	bool update(float dT);
	void draw();
	void leftClick();
	void leftClickPowerMode();

	void hitTarget();
	int getMissileDamage();

	void checkView(CHex& hex);

	CViewField viewField;

private:
	int tigCall(int memberId);
	void onNotify(COnNewHex& msg);

	void onNotify(CPlayerNewHex& msg);

	void deathRoutine();





};


