#pragma once

#define _USE_MATH_DEFINES //for cmath
#include <glm/glm.hpp>



#include "item2.h"

class CShieldComponent : public CItemCmp {
public:
	CShieldComponent(CEntity* parent) : CItemCmp(parent) {}
	void update(float dT);
	int absorbDamage(int damage);


	float hp = 20;
	float maxHp = 20;

	glm::vec4 chargedColour = { 0,0,1,0.15f };
	glm::vec4 flatColour = { 1,0,0,0.175f };

	float timeSinceHit = 0;
	float rechargeDelay = 4.0f;

	float timeRecharging = 0;
	float timeToRecharge = 2.0f;

	float rechargeRate = 7; //pts per second
	
};