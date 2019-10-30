#include "GUIdistributor.h"


CGUIdistributor::CGUIdistributor(int x, int y, int w, int h) : CGUIgamePanel(x, y, w, h) {
	availablePower = 15; offencePower = 0; defencePower = 0; remainingPower = availablePower;

	//add<CGUIbutton>("Button", 0);

	resizesForChildren.y = true;

	controlCursor.setCols(2);
	controlCursor.centreRow();

	CGUIlabel* headerNew = add<CGUIlabel>("Avl. power:", uiHright);

	powerLbl = add<CGUIlabel>("XX",uiHleft);

	controlCursor.setCols(1);
	headerNew = add<CGUIlabel>("Offence:",uiHcentred);

	controlCursor.setCols(2);
	offenceSlider = add<CGUIsysScrollbar2>(scrlHorizontal, uiHcentred);

	offenceID = offenceSlider->getUniqueID();
	offenceSlider->setMin(1); offenceSlider->setMax(100);

	offenceLbl = add<CGUIlabel>("YY", uiHcentred);

	controlCursor.setCols(1);
	headerNew = add<CGUIlabel>("Defence:", uiHcentred);

	controlCursor.setCols(2);

	defenceSlider = add<CGUIsysScrollbar2>(scrlHorizontal, uiHcentred);
	defenceID = defenceSlider->getUniqueID();
	defenceSlider->setMin(1); offenceSlider->setMax(100);

	defenceLbl = add<CGUIlabel>("ZZ", uiHcentred);
	
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
	powerLbl->setText(std::to_string((int)remainingPower));
	offenceLbl->setText(std::to_string((int)offencePower));
	defenceLbl->setText(std::to_string((int)defencePower));

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
	CMessage msg;
	msg = { uiMsgUpdate,offencePower,defencePower };
	callbackObj->GUIcallback(this, msg);
}
