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

	//transform = std::make_shared<CTransformCmp>(this);
	//modelCmp = std::make_shared<CModelCmp>(this);
}

CEntity::~CEntity() {
	//sysLog << "\nEntity " << id << " named " << name << " destructed.";
	onDestroy();

}

void CEntity::update(float dT) {
	this->dT = dT;
	diagnostic = "";

	for (auto& comp : components) {
		comp.second->update(dT);
	}
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
	for (auto& comp : components) {
		comp.second->thisEntity = this;
		comp.second->onSpawn();
	}
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
	//parentEntity = std::make_shared<CEntity>(*parent);
	parentEntity = parent;
}

CEntity* CEntity::getParent() {
	//return parentEntity.get();
	return parentEntity;
}

std::shared_ptr<CEntity> CEntity::getSmart() {
	return game.getEntitySmart(id);
}

void CEntity::removeComponent(CEntityCmp* component) {
	int id = component->getUniqueID();
	component->onRemove();
	components.erase(id);
}

void CEntity::onDestroy() {
	for (auto & component : components)
		component.second->onRemove();
	//components.clear();
}



//void CEntity::removePhysComponent() {
//	phys = nullptr;
//
//	CPhysicsEvent e;
//	e.entity = this;
//	e.action = physRemove;
//	lis::event<CPhysicsEvent>(e);
//}
//








