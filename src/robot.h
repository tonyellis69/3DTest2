#pragma once

#include <memory>

#include "gamehextObj.h"

#include "tigConst.h"

#include "defencePanels.h"

enum TPlayerDefence {defNone,defBlock,defEvade,defAbsorb,defFeint};



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
	void onMouseWheel(float delta);
	bool beginMove();
	void beginChasePlayer();
	void meleeAttackPlayer();
	void shootPlayer();

	int tigCall(int memberId);

	void spawnDefencePanel();


	TPlayerDefence playerDefence;
	CDefencePanel* defencePanel;
};


