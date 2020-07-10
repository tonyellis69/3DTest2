#include "fireable.h"

#include "utils/log.h"

#include "gameMsg.h"
#include "hexMsg.h"

CFireable::CFireable(const std::string& name) {
	tmpName = name;
	autoPower = autoNone;
	//powerId = 0;
}

/** Cycle the power acquisition mode of this firable. */
void CFireable::cycleAuto() {
	if (autoPower == autoBest)
		autoPower = autoNone;
	else
		autoPower = TAutoPower( autoPower + 1);


}

/** Return a string stating the given power acquisition setting. */
std::string CFireable::settingStr(TAutoPower autoPower) {
	switch (autoPower) {
	case autoNone: return "none";
	case autoLow: return "auto-low";
	case autoHigh: return "auto-high";
	case autoBest: return "auto-best";
	case autoCurrent: return "current";
	};
}

void CShield::cycleAuto(){
	CFireable::cycleAuto();

	//show the power this defence is going to get

	//it doesn't become a request until we actually fire it at a robot
	


}

/** Apply the current defence to this target. */
bool CShield::fireAt(CGameHexObj* target) {

	if (defences.find(target) != defences.end())
		return true; //lazy exit if defence already assigned

	CDefenceRec defenceRec;
	//defenceRec.autoSetting = autoPower;
	unsigned int id;
	int power = -1;
	if (autoPower == autoCurrent) {
		id = psu->reserveNextPower();
		power = psu->getPowerValue(id);
	}
	else
		id = psu->getBestPower(this);


	defenceRec.powerId = id;
	defenceRec.autoPower = autoPower;

	defences[target] = defenceRec;

	//file power request for this defence
	psu->requestPower(&defences[target]);

	liveLog << "\nshield of current power " + std::to_string(power) << " applied.";

	psu->updateDisplay();
	return true;
}

bool CShield::findDefendee(CGameHexObj* target) {
	for (auto defence : defences) {
		if (defence.first == target)
			return true;
	}
	return false;
}

std::string CShield::getUpdateText() {
	std::string txt = "Block\n" + settingStr(autoPower);

	if (autoPower == autoNone)
		return txt;

	unsigned int bestId = psu->getBestPower(this);


	//if (autoPower == autoCurrent) {
		int avail = psu->readNextPower();
		txt += "\nNext available power: " + std::to_string(avail);
		txt += "\nBest fit: " + std::to_string(psu->getPowerValue(bestId));
	
	//}
	return txt;
}

void CShield::onTurnEnd() {
	//assume each defence has used up its allotted power.
	for (auto& defence : defences) {	
		liveLog << "\nshield consumes id " << defence.second.powerId << " pow " << psu->getPowerValue(defence.second.powerId);
		psu->consumePower(defence.second.powerId);
		psu->updateDisplay();
		defence.second.powerId = 0;
	}
}

/** Remove any defence applied to this target. */
void CShield::cancelDefence(CGameHexObj* robot) {
	for (auto defence : defences) {
		if (defence.first == robot) {
			int id = defence.second.powerId;
			psu->unreservePower(id);
			psu->removeRequest((CPowerUser*)&defence.second);
			defences.erase(robot);
			return;
		}
	}
}

/** Mouse entered new hex. */
void CShield::onNotify(COnNewHex& msg) {
	//check if we have a defence on a robot at this hex. 
	for (auto defence : defences) {
		if (defence.first->hexPosition == msg.newHex) {
			std::string popupTxt = "Block defence assigned";
			popupTxt += "\nPower acquisition: ";
			popupTxt += settingStr(defence.second.autoPower);

			CSendText popMsg(defencePopup, popupTxt);
			send(popMsg);
			return;
		}
	}
	//no? Ensure it's turned off then
	//CPopupText popMsg(defencePopup, hidePopup);
	//send(popMsg);
}

CWeapon::CWeapon(const std::string& name) : CFireable(name) {
	fired = false;
}

void CWeapon::cycleAuto() {
	CFireable::cycleAuto();

	if (autoPower == autoNone) {
		psu->removeRequest(this);
		return;
	}

	//if the gun is set to any power setting other than none, it is requesting power
	psu->requestPower(this);

}


void CWeapon::loadPower() {
	if (powerId == 0)
		return;

	if (autoPower == autoCurrent) {
		powerId = psu->reserveNextPower();
	}
}

bool CWeapon::fireAt(CGameHexObj* target) {
	if (autoPower == autoNone) {
		liveLog << "\nNo power requested, not firing.";
		fired = false;
		return false;
	}

	//if (powerId == 0) { //we haven't acquired a power yet so get one now
	//	powerId = psu->getBestPower(this);
	//	//TO DO: run autoAssign instead, so that defences are assigned a new power 
	//	//if the gun takes theirs

	//}

	psu->autoAssign();
	
	
	int power = psu->consumePower(powerId); //acquire the power
	liveLog << "\nshot of " + std::to_string(power) << " fired."; 
	fired = true;
	powerId = 0;
	return false;
}

std::string CWeapon::getUpdateText() {
	std::string txt = "Gun \nPower setting: " + settingStr(autoPower);
	if (autoPower == autoNone)
		return txt;

	//if (autoPower == autoCurrent) {
		if (powerId == 0) {
			int avail = psu->readNextPower();
			txt += "\nNext available power: " + std::to_string(avail);
			int bestId = psu->getBestPower(this);
			txt += "\nBest fit: " + std::to_string(psu->getPowerValue(bestId));
		}
		else {
			int assigned = psu->getPowerValue(powerId);
			txt += "\nAssigned power: " + std::to_string(assigned);
		}
	//}
	return txt;
}

void CWeapon::onTurnEnd() {
	if (!fired && powerId > 0) { //gun never fired
		int power = psu->getPowerValue(powerId);
		psu->unreservePower(powerId);
		liveLog << "\nGun returned " << std::to_string(power);

	}
	fired = false; //reset for next turn
	powerId = 0;
}
