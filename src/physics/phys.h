#pragma once

#include <glm/glm.hpp>

#include "../entity/component.h"

/** Simple class for storing physics properties. */
class CPhys : public CDerivedC<CPhys> {
public:
	CPhys(CEntity* ent, float iMass) : CDerivedC(ent) {
		invMass = iMass;
	}
	void onAdd();
	void onRemove();

	glm::vec3 moveImpulse = { 0,0,0 }; //Motive force.
	glm::vec3 velocity = { 0,0,0 };
	float invMass = 0;
	float restitution = 0.0f; //bounce
	float drag = 0.00125f;

	float boundingRadius = 0;
};

