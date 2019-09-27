#pragma once

#include "UI/GUIcontrols.h"

#include "GUIgamePanel.h"


class CGUIdistributor : public CGUIgamePanel {
public:
	CGUIdistributor(int x, int y, int w, int h);
	void message(CGUIbase* sender, CMessage& msg);
	void updateDisplay();
	void setAvailablePower(int power);

	CGUIlabel2* powerLbl;
	CGUIsysScrollbar* offenceSlider;
	CGUIlabel2* offenceLbl;

	CGUIsysScrollbar* defenceSlider;
	CGUIlabel2* defenceLbl;

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
