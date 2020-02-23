#pragma once

#include "gamehextObj.h"

#include "tigConst.h"

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CGameHexObj {
public:
	CRobot();
	void chooseTurnAction();
	void beginTurnAction();
	bool postAction();
	bool update(float dT);
	void receiveDamage(CGameHexObj& attacker, int damage);

private:
	int attackOrNot();
	int tigCall(int memberId);

};


