#include "entity.h"

#include <string>
#include <memory>

#include "utils\log.h"

#include "..\3Dtest\src\hexRenderer.h"

#include <glm/gtc/matrix_transform.hpp>	

#include "../gameState.h"

#include "../hexRender/entityDraw.h"

#include "listen/listen.h"
#include "..\physEvent.h"

#include "../gameState.h"

const float rad360 = M_PI * 2;

unsigned int CEntity::nextId = 1;

CEntity::CEntity() {
	if (id == 0)
		id++;
	id = nextId++;

	transform = std::make_shared<CTransformCmp>(this);
	modelCmp = std::make_shared<CModelCmp>(this);
}

void CEntity::update(float dT) {
	this->dT = dT;
	diagnostic = "";

	if (tmpId == 57)
		int b = 0;

	//update various components here
	if (ai)
		ai->update(dT);
	if (item)
		item->update(dT);
	if (transform)
		transform->update(dT);  //TO DO: may not need
	if (modelCmp)
		modelCmp->update(dT);
	if (collider)
		collider->update(dT);
}


//
///** Set position using hex cube coordinates. */
//void CEntity::setPosition(CHex& hex) {
//	setPosition(cubeToWorldSpace(hex));
//}
//TO DO: phase out the old hex-based usage!

void CEntity::setPosition(glm::vec3& worldPos) {
	//hexPosition = worldSpaceToHex(worldPos);
//	destination = hexPosition;

	transform->setPos(worldPos);

}


void CEntity::onSpawn() {
	if (ai)
		ai->onSpawn();
	if (item)
		item->onSpawn();
	if (transform)
		transform->onSpawn();  //TO DO: may not need
	if (modelCmp)
		modelCmp->onSpawn();
	if (collider)
		collider->onSpawn();
	if (phys)
		collider->onSpawn();
}


glm::vec3 CEntity::getPos()
{
	return transform->worldPos;
}

void CEntity::destroyMe() {
	live = false;
	deleteMe = true;
	game.map->entitiesToKill = true;
}

void CEntity::setParent(CEntity* parent) {
	parentEntity = std::make_shared<CEntity>(*parent);
}

CEntity* CEntity::getParent() {
	return parentEntity.get();
}

void CEntity::addComponent(std::shared_ptr<CPhys> phys) {
	this->phys = phys;

	CPhysicsEvent e;
	e.entity = this;
	e.action = physAdd;
	lis::event<CPhysicsEvent>(e);
}

void CEntity::addAIComponent(std::shared_ptr<CAiCmp> ai) {
	this->ai = ai;
}

void CEntity::removePhysComponent() {
	phys = nullptr;

	CPhysicsEvent e;
	e.entity = this;
	e.action = physRemove;
	lis::event<CPhysicsEvent>(e);
}









