#include "GUIdistributor.h"

CGUIdistributor::CGUIdistributor(int x, int y, int w, int h) : CGUIgamePanel(x, y, w, h) {
	availablePower = 15; offencePower = 0; defencePower = 0; remainingPower = availablePower;

	//CGUIlabel* headerNew = (CGUIlabel *)add(uiLabel, "Available power:");

	CGUIlabel2* header = new CGUIlabel2(0, vSpace, 150, 20);
	header->setText("Available power:");
	header->hFormat = hCentre;
	Add(header);

	powerLbl = new CGUIlabel2(0, vSpace * 2, 30, 50);
	powerLbl->setText("XX");
	powerLbl->hFormat = hCentre;
	Add(powerLbl);

	header = new CGUIlabel2(0, vSpace * 4, 50, 50);
	header->setText("Offence:");
	header->hFormat = hCentre;
	Add(header);

	offenceSlider = new CGUIsysScrollbar(horizontal, hScrollbarOffset, vSpace * 7, scrollbarWidth);
	offenceID = offenceSlider->getUniqueID();
	offenceSlider->setMin(1); offenceSlider->setMax(100);
	Add(offenceSlider);

	offenceLbl = new CGUIlabel2(scrollbarWidth + hScrollbarOffset*2, vSpace * 7, 30, 18);
	offenceLbl->setText("YY");
	Add(offenceLbl);

	header = new CGUIlabel2(0, vSpace * 9, 60, 50);
	header->setText("Defence:");
	header->hFormat = hCentre;
	Add(header);

	defenceSlider = new CGUIsysScrollbar(horizontal, hScrollbarOffset, vSpace * 12, scrollbarWidth);
	defenceID = defenceSlider->getUniqueID();
	defenceSlider->setMin(1); offenceSlider->setMax(100);
	Add(defenceSlider);

	defenceLbl = new CGUIlabel2(scrollbarWidth + hScrollbarOffset * 2, vSpace * 12, 30, 18);
	defenceLbl->setText("ZZ");
	Add(defenceLbl);
	
	updateDisplay();
}


void CGUIdistributor::message(CGUIbase* sender, CMessage& msg) {
	if (msg.Msg == uiMsgSlide) {
		if (sender->getUniqueID() == offenceID) {
			//find the portion of offence power
			//take that from the available power
			//reduce defence power if there's not enough left for its portion
			float proportion = offenceSlider->Value / 100.0f;
			offencePower = availablePower * proportion;
			remainingPower = availablePower - offencePower - defencePower;
			if (remainingPower < 0) {
				defencePower += remainingPower;
				defenceSlider->setValue((defencePower * 100.0f) / availablePower);
				remainingPower = 0;
			}
		}
		else {
			float proportion = defenceSlider->Value / 100.0f;
			defencePower = availablePower * proportion;
			remainingPower = availablePower - offencePower - defencePower;
			if (remainingPower < 0) {
				offencePower += remainingPower;
				offenceSlider->setValue((offencePower * 100.0f) / availablePower);
				remainingPower = 0;
			}

		}


		updateDisplay();
		CMessage msg;
		msg = { uiMsgUpdate,offencePower,defencePower };
		callbackObj->GUIcallback(this, msg);
	}
}

/** Update the display to show the current distribution of power. */
void CGUIdistributor::updateDisplay() {
	powerLbl->setText(to_string((int)remainingPower));
	offenceLbl->setText(to_string((int)offencePower));
	defenceLbl->setText(to_string((int)defencePower));

	needsUpdate = true;

}

/** Set the amount of power available for distribution and update the current
	distribution. */
void CGUIdistributor::setAvailablePower(int power) {
	availablePower = power;
	offencePower = (offenceSlider->Value / 100.0f) * availablePower;
	defencePower = (defenceSlider->Value / 100.0f) * availablePower;
	remainingPower = availablePower - offencePower - defencePower;
	updateDisplay();
}
