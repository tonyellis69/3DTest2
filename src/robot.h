#pragma once

#include <memory>

#include "gamehextObj.h"

#include "tigConst.h"

#include "shieldPanel.h"

#include "gameMsg.h"


/** A class describing basic robot characteristics and
	behaviour. */
class CRobot : public CGameHexObj {
public:
	CRobot();
	void chooseTurnAction();
	bool update(float dT);
	void receiveDamage(CGameHexObj& attacker, int damage);
	void draw();
	void onDefenceClick();
	TPlayerDefence getDefence();
	void cycleDefence();
	TDefence getDefenceSettings();
	void setDefenceSettings(TDefence& newSettings);


private:
	void initialiseCurrentAction();
	bool onLeftClick();
	void onMouseOverNorm();
	void onMouseWheel(float delta);
	void onNewMouseHex(CHex& mouseHex);
	bool beginMove();
	void beginChasePlayer();
	void meleeAttackPlayer();
	void shootPlayer();

	int tigCall(int memberId);

	void onNotify(COnNewHex& msg);

	TPlayerDefence defence;

	TDefence defenceSettings;

};


