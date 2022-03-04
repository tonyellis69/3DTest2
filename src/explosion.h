#pragma once

#include "entity.h"

/** Prototype explosion sprite. */
class CExplosion : public CEntity {
public:
	CExplosion(float size);
	void update(float dT);
	void draw();
	void setCollidee(CEntity* collidee);

	void initDrawFn();

	float lifeTime = 0;
	float timeOut = 3.0f;
	float size;
	int particleCount;
	CEntity* collidee = nullptr; ///<Entity hit, if any.
	glm::vec3 relativePos; ///<From collidee.

	const float maxSize = 10;
	float seed; 
};