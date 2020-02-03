#pragma once

#include "hex/hexObject.h"

/** A class describing characteristics and behaviour unique to
	the player hex-world object. */
class CPlayerObject : public CHexObject {
public:
	CPlayerObject();
	bool update(float dT);
	void beginAttack(CHexObject& target);
	void receiveDamage(CHexObject& attacker, int damage);
	void draw();
	void setShield(THexDir shieldDir);

	CBuf* shieldBuf;
	
};