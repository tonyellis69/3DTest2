#pragma once

#include "gamehextObj.h"

/** A class describing characteristics and behaviour unique to
	the player hex-world object. */
class CPlayerObject : public CGameHexObj {
public:
	CPlayerObject();
	bool update(float dT);
	void beginAttack(CGameHexObj& target);
	void hitTarget();
	void receiveDamage(CGameHexObj& attacker, int damage);
	void draw();
	void setShield(THexDir shieldDir);
	void takeItem(CGameHexObj& item);
	void showInventory();
	void dropItem(int itemNo);
	void equipItem(int itemNo);

	CBuf* shieldBuf;
	
};