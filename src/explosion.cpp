#include "explosion.h"

#include "gameState.h"
#include "hexRenderer.h"


CExplosion::CExplosion(float size) {
	this->size = size;
	timeOut = glm::mix(timeOut, 0.3f,1 - size / 10.0f);
}

void CExplosion::update(float dT) {
	lifeTime += dT;

	if (lifeTime > timeOut) {
		world.destroySprite(*this);
		//lifeTime = timeOut;

	}
}

void CExplosion::draw() {
	hexRendr2.drawExplosion(worldPos, lifeTime, size, timeOut);
}
