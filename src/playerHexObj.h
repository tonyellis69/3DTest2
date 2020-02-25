#pragma once

#include "gamehextObj.h"

/** A class describing characteristics and behaviour unique to
	the player hex-world object. */
class CPlayerObject : public CGameHexObj {
public:
	CPlayerObject();
	bool update(float dT);
	void beginAttack(CGameHexObj& target);
	void receiveDamage(CGameHexObj& attacker, int damage);
	void draw();
	void setShield(THexDir shieldDir);

	CBuf* shieldBuf;
	
};