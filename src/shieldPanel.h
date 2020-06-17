#pragma once

#include "gameTextWin.h"
#include "fireable.h"

enum TPlayerDefence { defNone, defBlock, defEvade, defAbsorb, defFeint };


struct TDefence {
	TPlayerDefence defence;
	int power = 1;
	int processing = 1;

};

class CRobot;
class IShield {
public:
	virtual void robotClick(CRobot* robot) = 0;
};



/** Temporary class for prototyping the shield/defence control. */
class CFireablePanel : public CGameTextWin, public IShield {
public:
	CFireablePanel();
	void robotClick(CRobot* robot);
	void setFireable(CFireable* fireable);

	bool MouseWheelMsg(const  int mouseX, const  int mouseY, int wheelDelta, int key);

	void cycleAuto();
	void updateDisplay();
	void loadPower();

	CFireable* fireable;
	TAutoPower shieldAuto; ///<Power acquisition setting
	TAutoPower gunAuto;
};

