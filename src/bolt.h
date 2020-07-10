#pragma once

#include "gridObj.h"

#include "messaging/messenger.h"

#include "gamehextObj.h"

/** Class describing a simple missile object. */
class CBolt : public CGridObj , public CMessenger {
public:
	CBolt();
	void fireAt(CGameHexObj* attacker, CHex& target);
	bool update(float dT);

	glm::vec3 targetPos;
	CHex targetHex;
	CGameHexObj* attacker;
};


class CMissileHit : public CMsg {
public:
	CMissileHit(CHex& h, CGameHexObj* a) :
		hex(h), attacker(a) {}

	CHex hex;
	CGameHexObj* attacker;

};