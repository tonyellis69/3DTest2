#pragma once

#include "UI/GUIcontrols.h"

#include "GUIgamePanel.h"

#include "UI/GUIscrollbar2.h"


class CGUIdistributor : public CGUIgamePanel {
public:
	CGUIdistributor(int x, int y, int w, int h);
	void message(CGUIbase* sender, CMessage& msg);
	void updateDisplay();
	void setAvailablePower(int power);

	CGUIlabel* powerLbl;
	CGUIsysScrollbar2* offenceSlider;
	CGUIlabel* offenceLbl;

	CGUIsysScrollbar2* defenceSlider;
	CGUIlabel* defenceLbl;

	int availablePower;
	int offencePower;
	int defencePower;
	int remainingPower;

	int offenceID;
	int defenceID;

};

const int vSpace = 15;
const int hScrollbarOffset = 20;
const int scrollbarWidth = 80;
