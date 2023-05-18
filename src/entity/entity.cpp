#include "entity.h"

#include <string>
#include <memory>

#include "utils\log.h"

//#include "..\3Dtest\src\hexRenderer.h"

#include <glm/gtc/matrix_transform.hpp>	

#include "../gameState.h"

#include "../hexRender/entityDraw.h"

#include "listen/listen.h"
#include "..\physEvent.h"

#include "../gameState.h"

#include <typeinfo>

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
	if (playerC)
		playerC->update(dT);
	if (healthC)
		healthC->update(dT);

	//for (auto& comp : components) {
	//	comp.second->update(dT);
	//}
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


void CEntity::init() {
	if (ai) {
		ai->thisEntity = this;
		ai->onSpawn();
	}
	if (item) {
		item->thisEntity = this;
		item->onSpawn();
	}
	if (transform) {
		transform->thisEntity = this;
		transform->onSpawn();  //TO DO: may not need
	}
	if (modelCmp) {
		modelCmp->thisEntity = this;
		modelCmp->onSpawn();
	}
	if (collider) {
		collider->thisEntity = this;
		collider->onSpawn();
	}
	if (phys) {
		phys->thisEntity = this;
		phys->onSpawn();
	}
	if (playerC) {
		playerC->thisEntity = this;
		playerC->onSpawn();
	}
	if (healthC) {
		healthC->thisEntity = this;
		healthC->onSpawn();
	}

	//for (auto& comp : components) {
	//	comp.second->thisEntity = this;
	//	comp.second->onSpawn();
	//}
}


glm::vec3 CEntity::getPos()
{
	return transform->worldPos;
}

void CEntity::destroyMe() {
	live = false;
	deleteMe = true;
	game.entitiesToKill = true;
}

void CEntity::setParent(CEntity* parent) {
	parentEntity = std::make_shared<CEntity>(*parent);
}

CEntity* CEntity::getParent() {
	return parentEntity.get();
}



void CEntity::addComponent(std::shared_ptr<CPhys> phys) {
	this->phys = phys;

	//CPhysicsEvent e;
	//e.entity = this;
	//e.action = physAdd;
	//lis::event<CPhysicsEvent>(e);
}

void CEntity::addComponent(std::shared_ptr<CPlayerC> playerC) {
	this->playerC = playerC;
}


void CEntity::addAIComponent(std::shared_ptr<CAiCmp> ai) {
	this->ai = ai;
}

void CEntity::addComponent(std::shared_ptr<CBotHealthC> healthC) {
	this->healthC = healthC;
}

void CEntity::addComponent(std::shared_ptr<CPlayerHealthC> healthC) {
	this->healthC = healthC;
}

void CEntity::removePhysComponent() {
	phys = nullptr;

	CPhysicsEvent e;
	e.entity = this;
	e.action = physRemove;
	lis::event<CPhysicsEvent>(e);
}









