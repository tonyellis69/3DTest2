#include "shield.h"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>	


#include "../entity/entity.h"


#include <glm/gtx/rotate_vector.hpp>


void CShieldComponent::update(float dT) {
	const float rad90 = float(M_PI) / 2;

	CTransformCmp* transform = thisEntity->transform;

	transform->setPos(itemOwner->getPos());
	transform->setRotation(rad90);
	transform->setScale(glm::vec3{ 0.75f });


	timeSinceHit += dT;
	if (timeSinceHit >= rechargeDelay) {
		timeSinceHit = rechargeDelay;
		if (hp < maxHp ) {
			hp += rechargeRate * dT;
			if (hp > maxHp)
				hp = maxHp;
		}
	}

	float damageRatio = hp / maxHp;
	glm::vec4 shieldColour = glm::mix(flatColour, chargedColour, damageRatio);
	thisEntity->modelCmp->model.palette[0] = shieldColour;

}

int CShieldComponent::absorbDamage(int damage) {
	timeSinceHit = 0;
	timeRecharging = 0;

	if (hp <= 0)
		return damage;

	int oldHp = hp;

	hp -= damage;
	if (hp < 0)
		hp = 0;

	if (hp > 0)
		return 0;
	return damage - oldHp;
}
