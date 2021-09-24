#include "explosion.h"

#include "gameState.h"
#include "hexRenderer.h"

#include "utils/log.h"
#include "utils/random.h"


CExplosion::CExplosion(float size) {
	this->size = size;
	float unitSize = (size - 1) / (maxSize - 1); //put in range 0-1
	timeOut = glm::mix(0.5f, timeOut, unitSize);
	float weighted = pow(unitSize, 0.35f);
	particleCount = glm::mix(20, hexRendr2.numExplosionParticles, weighted);
	seed = rnd::rand(1.0f);
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
	TExplode exp = { worldPos,lifeTime, size, timeOut, particleCount, seed };
	hexRendr2.drawExplosion(exp);
}

/** If we hit a moving target, this allows us to move with it. */
void CExplosion::setCollidee(CEntity* collidee) {
	this->collidee = collidee;
	relativePos = worldPos - collidee->worldPos;
}
