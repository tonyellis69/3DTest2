#pragma once

#include "healthC.h"


class CPlayerHealthC : public CHealthC {
public:
	CPlayerHealthC(CEntity* parent) : CHealthC(parent) {}
	void receiveDamage(CEntity& attacker, int damage);



};