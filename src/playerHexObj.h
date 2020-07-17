#pragma once

#include "hexItem.h"
#include "actor.h"

#include "powerSupply.h"

/** A class describing characteristics and behaviour unique to
	the player hex-world object. */
class CPlayerObject : public CHexActor {
public:
	CPlayerObject();
	void initAction();
	bool update(float dT);
	void hitTarget();
	void draw();
	void takeItem(CGameHexObj& item);
	void showInventory();
	void dropItem(int itemNo);
	void equipItem(int itemNo);
	void leftClickPowerMode();
	void onTurnBegin();
	void onTurnEnd();
	void onGetPlayerPos(CGetPlayerPos& msg);
	void onSetPlayerAction(CSetPlayerAction& msg);
	void onTakeItem(CTakeItem& msg);
	void onGetPlayerObj(CGetPlayerObj& msg);

	void deathRoutine();

	void receiveDamage(CGameHexObj& attacker, int damage);

	int getMissileDamage();


	TEntities playerItems; ///<Items temporarily taken out of hex world by player
	
	CPowerSupply* psu;

private:

};