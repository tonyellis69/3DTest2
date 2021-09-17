#include "explosion.h"

#include "gameState.h"
#include "hexRenderer.h"

#include "utils/log.h"


CExplosion::CExplosion(float size) {
	this->size = size;
	timeOut = glm::mix(timeOut, 0.3f,1 - size / 10.0f);
	entityType = entExplode;
}

void CExplosion::update(float dT) {
	lifeTime += dT;

	if (collidee) {
		worldPos = collidee->worldPos + relativePos;
	}

	if (lifeTime > timeOut) {
		game.deleteEntity(*this);
	}
}

void CExplosion::draw() {
	hexRendr2.drawExplosion(worldPos, lifeTime, size, timeOut);
}

/** If we hit a moving target, this allows us to move with it. */
void CExplosion::setCollidee(CEntity* collidee) {
	this->collidee = collidee;
	relativePos = worldPos - collidee->worldPos;
}
