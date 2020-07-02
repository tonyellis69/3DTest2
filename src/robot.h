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
	void receiveDamage(CGameHexObj& attacker, int damage);
	void draw();
	void leftClick();

private:
	int tigCall(int memberId);
	void onNotify(COnNewHex& msg);





};


