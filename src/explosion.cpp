#include "explosion.h"

#include "gameState.h"
#include "hexRenderer.h"


CExplosion::CExplosion(float size) {
	this->size = size;
	timeOut = glm::mix(timeOut, 0.3f,1 - size / 10.0f);
	entityType = entExplode;
}

void CExplosion::update(float dT) {
	lifeTime += dT;

	if (lifeTime > timeOut) {
		//world.destroySprite(*this);
		game.deleteEntity(*this);
		//lifeTime = timeOut;

	}
}

void CExplosion::draw() {
	hexRendr2.drawExplosion(worldPos, lifeTime, size, timeOut);
}
