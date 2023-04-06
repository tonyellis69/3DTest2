#pragma once

#include "healthC.h"


class CBotHealthC : public CHealthC {
public:
	CBotHealthC(CEntity* parent) : CHealthC(parent) {}
	void receiveDamage(CEntity& attacker, int damage);



};