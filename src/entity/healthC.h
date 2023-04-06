#pragma once

#include "component.h"


class CHealthC : public CEntityCmp {
public:
	CHealthC(CEntity* parent) : CEntityCmp(parent) {}
	virtual void receiveDamage(CEntity& attacker, int damage) {}


	int hp;

};