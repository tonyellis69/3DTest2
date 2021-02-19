#pragma once

#include "sprite.h"

/** Prototype explosion sprite. */
class CExplosion : public CSprite {
public:
	CExplosion(float size);
	void update(float dT);
	void draw();

	float lifeTime = 0;
	float timeOut = 2.0f;
	float size;
};