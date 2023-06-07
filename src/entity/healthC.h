#pragma once

#include "component.h"
#include "healthC.h"


class CHealthC : public CDerivedC<CHealthC> {
public:
	CHealthC(CEntity* parent) : CDerivedC(parent) {}
	void onAdd();
	void onRemove();
	virtual void receiveDamage(CEntity& attacker, int damage) {}


	int hp;

};