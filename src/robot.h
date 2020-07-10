#pragma once

#include "tigConst.h"

#include "gameMsg.h"

#include "actor.h"


/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CHexActor {
public:
	CRobot();
	void chooseTurnAction();
	void draw();
	void leftClick();
	void leftClickPowerMode();

	void hitTarget();

private:
	int tigCall(int memberId);
	void onNotify(COnNewHex& msg);

	void deathRoutine();





};


