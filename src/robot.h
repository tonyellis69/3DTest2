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
	bool update(float dT);
	void receiveDamage(CGameHexObj& attacker, int damage);

private:
	bool onLeftClick();
	void beginChasePlayer();
	void meleeAttackPlayer();
	void shootPlayer();
	bool afterResolving();

	int tigCall(int memberId);

};


