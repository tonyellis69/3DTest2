#include "GUIhealthPanel.h"

CGUIhealthPanel::CGUIhealthPanel(int x, int y, int w, int h) : CGUIgamePanel(x, y, w, h) {
	resizesForChildren.y = true;

	controlCursor.setCols(2);
	//controlCursor.centreRow();

	CGUIlabel* lbl = add<CGUIlabel>("Cell power:", uiHright);
	powerLbl = add<CGUIlabel>("XX", uiHleft);

	//controlCursor.setCols(1);
	lbl = add<CGUIlabel>("Hit points:", uiHright);
	hpLbl = add<CGUIlabel>("XX", uiHleft);



}

void CGUIhealthPanel::message(CGUIbase* sender, CMessage& msg) {

}

void CGUIhealthPanel::updateDisplay() {
	powerLbl->setText(std::to_string((int)power));
	hpLbl->setText(std::to_string((int)HP));
	needsUpdate = true;
}

void CGUIhealthPanel::setHP(int newHP) {
	HP = newHP;
	updateDisplay();
}

void CGUIhealthPanel::setPower(int newPower) {
	power = newPower;
	updateDisplay();
}
