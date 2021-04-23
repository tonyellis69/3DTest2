#pragma once

#include <unordered_set>
#include <set>

#include "sprite.h"

#include "hex/hex.h"
#include "gamehextObj.h"

using TIntersections = std::vector<std::pair<CHex, glm::vec3>>;

/** Prototype realtime missile. */
class CMissile : public CSprite {
public:
	CMissile();
	void setPosition(glm::vec3& pos, float rotation);
	void update(float dT);
	void draw();
	void setOwner(CGameHexObj* owner);

private:
	void approachDestHex();
	bool collisionCheck(glm::vec3& moveVec);

	void spawnExplosion();

	glm::vec3 startingPos;
	glm::vec3 dirVec;
	float missileMoveSpeed = 30.0f;

	glm::vec3 leadingPoint; ///<Point we test for collisions.
	glm::vec3 leadingPointLastHex;
	float distToPoint = 0.1f;
	CHex lastLeadingPointHex;

	bool collided = false;

	CGameHexObj* owner;

	glm::vec3 collisionPt;

	TIntersections intersectedHexes;
};