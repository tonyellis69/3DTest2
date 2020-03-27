#pragma once

#include "gamehextObj.h"

#include "tigConst.h"

/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CGameHexObj {
public:
	CRobot();
	void chooseTurnAction();
	bool update(float dT);
	void receiveDamage(CGameHexObj& attacker, int damage);

private:
	void initialiseCurrentAction();
	bool onLeftClick();
	void beginChasePlayer();
	void meleeAttackPlayer();
	void shootPlayer();


	int tigCall(int memberId);

};


