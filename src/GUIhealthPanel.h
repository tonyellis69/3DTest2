#pragma once

#include "UI/GUIcontrols.h"

#include "GUIgamePanel.h"



class CGUIhealthPanel : public CGUIgamePanel {
public:
	CGUIhealthPanel(int x, int y, int w, int h);
	void message(CGUIbase* sender, CMessage& msg);
	void updateDisplay();
	void setHP(int newHP);
	void setPower(int newPower);

	CGUIlabel* powerLbl;
	CGUIlabel* hpLbl;

	int HP;
	int power;

};