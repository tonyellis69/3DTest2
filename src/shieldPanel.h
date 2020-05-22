#pragma once

#include "gameTextWin.h"

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
class CShieldPanel : public CGameTextWin, public IShield {
public:
	CShieldPanel();
	void robotClick(CRobot* robot);
	void onRelease();
	void showCurrentRobotDefence();
	void cycleCurrentRobotDefence();

	bool MouseWheelMsg(const  int mouseX, const  int mouseY, int wheelDelta, int key);


	CRobot* currentRobot;
	
};

