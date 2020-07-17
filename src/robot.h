#pragma once
#define _USE_MATH_DEFINES

#include "tigConst.h"

#include "gameMsg.h"

#include "actor.h"



/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CHexActor {
public:
	CRobot();
	void chooseTurnAction();
	bool update(float dT);
	void draw();
	void leftClick();
	void leftClickPowerMode();

	void hitTarget();
	int getMissileDamage();

	TFov fov;

private:
	int tigCall(int memberId);
	void onNotify(COnNewHex& msg);

	void deathRoutine();





};


