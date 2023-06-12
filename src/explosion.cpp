#include "explosion.h"

#include "gameState.h"
//#include "hexRenderer.h"

#include "utils/log.h"
#include "utils/random.h"

#include "hexRender/splodeDraw.h"

CExplosion::CExplosion(float size) {
	this->size = size;
	float unitSize = (size - 1.0f) / (maxSize - 1); //put in range 0-1
	timeOut = glm::mix(0.75f, timeOut, unitSize);
	float weighted = pow(unitSize, 0.35f);
	particleCount = glm::mix(20, numExplosionParticles, weighted);
	seed = rnd::rand(1.0f);
	entityType = entExplode;
}

void CExplosion::update(float dT) {
	lifeTime += dT;

	if (collidee) {
		transform->setPos(collidee->getPos() + relativePos);
	}
	//error here: collidee out of scope

	if (lifeTime > timeOut) {
		gameWorld.deleteEntity(*this);
	}
}



/** If we hit a moving target, this allows us to move with it. */
void CExplosion::setCollidee(std::shared_ptr<CEntity> collidee) {
	this->collidee = collidee;
	//this->collidee = std::make_shared<CEntity>(*collidee);
	relativePos = getPos() - collidee->getPos();
}



