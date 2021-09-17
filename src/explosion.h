#pragma once

#include "entity.h"

/** Prototype explosion sprite. */
class CExplosion : public CEntity {
public:
	CExplosion(float size);
	void update(float dT);
	void draw();
	void setCollidee(CEntity* collidee);

	float lifeTime = 0;
	float timeOut = 1.0f;
	float size;
	CEntity* collidee = nullptr; ///<Entity hit, if any.
	glm::vec3 relativePos; ///<From collidee.
};