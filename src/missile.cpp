#include "missile.h"

#include <glm/gtc/matrix_transform.hpp>

#include "gameState.h"
#include "utils/log.h"

//#include "hexRenderer.h"

#include "explosion.h"

CMissile::CMissile() {
	entityType = entMissile;
}

void CMissile::setPosition(glm::vec3& pos, float rotation) {
	//CSprite::setPosition(pos, rotation);

	//worldPos = pos;
	transform->setPos(pos);
	transform->setRotation(rotation);

	//modelCmp->translateAll(getPos());
	//modelCmp->rotate(transform->rotation);

	//dirVec =  glm::normalize(modelCmp->model.getMainMesh()->matrix * glm::vec4(1, 0, 0, 0));
	dirVec =  { cos(rotation), -sin(rotation),0 }; 

	leadingPoint = pos + dirVec * distToPoint;
	leadingPointLastHex = leadingPoint;
	lastLeadingPointHex = worldSpaceToHex(pos);
	startingPos = pos;
}

void CMissile::update(float dT) {
	CEntity::update(dT);
	if (collided) {
		spawnExplosion();
		destroyMe();
		return;
	}
	this->dT = dT;
	approachDestHex();
}

void CMissile::draw() {
	//hexRendr2.drawLineModel(model.meshes[0]);
}

void CMissile::setOwner(CEntity* owner) {
	this->owner = owner;
}

void CMissile::setSpeed(float speed) {
	missileMoveSpeed = speed;
}


/** Move realtime in the current  direction. */
void CMissile::approachDestHex() {
	glm::vec3 moveVec = dirVec * missileMoveSpeed * 1000.0f;

	phys->moveImpulse = moveVec ;



//	glm::vec3 currentWPos = transform->worldPos;
//	transform->setPos(getPos() + moveVec);


	leadingPoint = getPos() + dirVec * distToPoint;

	//collisionCheck(moveVec);

}


void CMissile::spawnExplosion() {
	CExplosion* splode = (CExplosion *) game.spawn("explosion", collisionPt, 1);
	if (collidee)
		splode->setCollidee(collidee->getSmart());

}


