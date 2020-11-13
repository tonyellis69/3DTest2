#pragma once

#include "sprite.h"

#include "hex/hex.h"

/** Prototype realtime missile. */
class CMissile : public CSprite {
public:
	CMissile();
	void setPosition(glm::vec3& pos, float rotation);
	void update(float dT);
	void draw();

private:
	void moveReal();
	bool collisionCheck(glm::vec3& moveVec);

	glm::vec3 dirVec;
	float missileMoveSpeed = 30.0f;

	glm::vec3 leadingPoint; ///<Point we test for collisions.
	glm::vec3 leadingPointLastHex;
	float distToPoint = 0.1f;
	CHex lastLeadingPointHex;

	bool collided = false;
};