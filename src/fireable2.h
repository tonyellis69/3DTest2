#pragma once

#include <string>
#include <map>

#include "gamehextObj.h"

//#include "messaging/events.h"
#include "messaging/messenger.h"
#include "gameEvents.h"

#include "powerUser.h"
#include "powerSupply.h"


/** Basic thing-that-uses-power class. */
class CFireable : public CPowerUser, public CGameEventObserver,
	public CMessenger {
public:
	//CFireable() { };
	CFireable(const std::string& name) ;
	virtual bool fireAt(CGameHexObj* target) = 0;
	virtual void cycleAuto();
	virtual void loadPower() {};
	virtual std::string getUpdateText() = 0;
	std::string settingStr(TAutoPower autoPower);
	virtual void onTurnEnd() = 0;

	std::string tmpName;
	//TAutoPower autoPower; ///<Power acquisition mode

	static inline CPowerSupply* psu;


};


class CDefenceRec : public CPowerUser {
public:
	//CGameHexObj* attacker;
	//TAutoPower autoSetting;
	//unsigned int powerId;
};

class CShield : public CFireable {
public:
	CShield(const std::string& name) : CFireable(name) {}
	void cycleAuto();

	bool fireAt(CGameHexObj* target);

	bool findDefendee(CGameHexObj* target);

	std::string getUpdateText();
	void onTurnEnd();

	void cancelDefence(CGameHexObj* robot);


	void onNotify(COnCursorNewHex& msg);

	std::map<CGameHexObj*, CDefenceRec> defences;
};


class CWeapon : public CFireable {
public:
	CWeapon(const std::string& name);
	void cycleAuto();
	void loadPower();
	
	bool fireAt(CGameHexObj* target);

	std::string getUpdateText();

	void onTurnEnd();

	bool fired; ///<Gun fired this round.
};