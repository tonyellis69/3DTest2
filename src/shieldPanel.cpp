#include "shieldPanel.h"

#include "robot.h"

//TPlayerDefence& operator++(TPlayerDefence& def, int) {
//	return def = (def == defFeint) ? defNone : TPlayerDefence(def + 1);
//}

CFireablePanel::CFireablePanel()  {
	resize(300, 150);
	setLocalPos(500, 20);
	anchorRight = 10;

	addText("Here is some default text. ");



}

void CFireablePanel::robotClick(CRobot* robot) {
	

}

void CFireablePanel::setFireable(CFireable* fireable) {
	this->fireable = fireable;
	clearText();
	addText(fireable->getUpdateText());
}




bool CFireablePanel::MouseWheelMsg(const int mouseX, const int mouseY, int wheelDelta, int key) {
	

	return true;
}

/** Cycle the auto power setting of the current fireable. */
void CFireablePanel::cycleAuto() {
	fireable->cycleAuto();
	//clearText();
	//addText(fireable->getUpdateText());
	updateDisplay();
}


void CFireablePanel::updateDisplay() {
	clearText();
	addText(fireable->getUpdateText());
}

/** Load the next available power into this fireable. */
void CFireablePanel::loadPower() {
	fireable->loadPower();
}
