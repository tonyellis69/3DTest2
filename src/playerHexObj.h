#pragma once

#include "gamehextObj.h"

/** A class describing characteristics and behaviour unique to
	the player hex-world object. */
class CPlayerObject : public CGameHexObj {
public:
	CPlayerObject();
	void stackAction(CAction chosenAction);
	bool update(float dT);
	void beginLunge(CGameHexObj& target);
	void hitTarget();
	void receiveDamage(CGameHexObj& attacker, int damage);
	void draw();
	void setShield(THexDir shieldDir);
	void takeItem(CGameHexObj& item);
	void showInventory();
	void dropItem(int itemNo);
	void equipItem(int itemNo);
	void fireShot(CHex& target);

	CBuf* shieldBuf;
	CLineModel shieldModel;
	TEntities playerItems; ///<Items temporarily taken out of hex world by player

private:
	void initialiseCurrentAction();
};