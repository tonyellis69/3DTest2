#include "shieldPanel.h"

#include "robot.h"

//TPlayerDefence& operator++(TPlayerDefence& def, int) {
//	return def = (def == defFeint) ? defNone : TPlayerDefence(def + 1);
//}

CShieldPanel::CShieldPanel()  {
	resize(300, 100);
	setLocalPos(500, 20);
	anchorRight = 10;

	addText("Here is some default text. ");

	currentRobot = NULL;
}

void CShieldPanel::robotClick(CRobot* robot) {
	//if we're invisible:
	//if this robot has a defence, switch to that
	//if not, switch to block
	//go visible

	//if we're visible:
	//if we're already displaying this robot's defence, cycle to the next
	//if not, switch to this robot's defence

	if (!visible) {
		currentRobot = robot;
		showCurrentRobotDefence();
		setVisible(true);
	}
	else {
		if (currentRobot == robot)
			cycleCurrentRobotDefence();
		else {
			currentRobot = robot;
			showCurrentRobotDefence();
		}

	}

}


void CShieldPanel::onRelease() {
	visible = false;
	currentRobot = NULL;
}

void CShieldPanel::showCurrentRobotDefence() {
	clearText();
	std::string name = currentRobot->callTigStr(tig::name);
	addText(name);

	TDefence settings;

	switch (currentRobot->getDefence()) {
	case defNone: addText("\nBlock"); currentRobot->cycleDefence(); break;
	case defBlock : addText("\nBlock"); break;
	case defEvade: addText("\nEvade"); break;
	case defAbsorb: addText("\nAbsorb"); break;
	case defFeint: addText("\nFeint"); break;
	}

	settings = currentRobot->getDefenceSettings();
	std::string settingTxt = "\nPower " + std::to_string(settings.power);
	addText(settingTxt);
}

void CShieldPanel::cycleCurrentRobotDefence() {
	currentRobot->cycleDefence();
	if (currentRobot->getDefence() == defNone) {
		setVisible(false);
		currentRobot = NULL;
		return;
	}
	showCurrentRobotDefence();
}

bool CShieldPanel::MouseWheelMsg(const int mouseX, const int mouseY, int wheelDelta, int key) {
	TDefence settings = currentRobot->getDefenceSettings();
	settings.power += wheelDelta;
	currentRobot->setDefenceSettings(settings);
	showCurrentRobotDefence();

	return true;
}
