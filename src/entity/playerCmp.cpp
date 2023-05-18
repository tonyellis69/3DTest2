#include "playerCmp.h"

#include <glm/glm.hpp>

#include "entity.h"

#include "../gameState.h"

#include "../items/gun.h"
#include "../gameGui.h"

//#include "../playerHexObj.h"
#include "../items/armour.h"
#include "../items/shield.h"


constexpr float sin30 = 0.5f;
constexpr float sin60 = 0.86602540378443;

CPlayerC::CPlayerC(CEntity* parent) : CDerivedC(parent) {

}

void CPlayerC::onSpawn()
{
	shield = game.getEntity(shieldId);
}

void CPlayerC::onFireKey(bool pressed) {
	if (!pressed || dead || gun == nullptr)
		return;

	gun->fire(thisEntity->transform->getUpperBodyRotation());
}


/** Respond to player move instruction. */
void CPlayerC::moveCommand(TMoveDir commandDir) {
//	((CPlayerObject*)thisEntity)->moveCommand(commandDir);
//	return;

	if (dead)
		return;


	const float accel = 4000;
	switch (commandDir) {
	case moveNorth:  thisEntity->phys->moveImpulse = { 0,1,0 }; break;
	case moveNE: thisEntity->phys->moveImpulse = { sin30, sin60, 0 }; break;
	case moveEast: thisEntity->phys->moveImpulse = { 1,0,0 }; break;
	case moveSE: thisEntity->phys->moveImpulse = { sin30, -sin60, 0 }; break;
	case moveSouth: thisEntity->phys->moveImpulse = { 0,-1,0 }; break;
	case moveSW: thisEntity->phys->moveImpulse = { -sin30, -sin60, 0 }; break;
	case moveWest: thisEntity->phys->moveImpulse = { -1,0,0 }; break;
	case moveNW: thisEntity->phys->moveImpulse = { -sin30, sin60, 0 }; break;
	}


	moveDir = commandDir;


	//turn to that direction
	walkingBackwards = false;
	//glm::vec3 upperRotation = getUpperBodyRotationVec();
	glm::vec3 upperRotation = thisEntity->transform->getUpperBodyRotationVec();
	if (glm::dot(thisEntity->phys->moveImpulse, upperRotation) >= 0) {
		thisEntity->transform->setRotation(thisEntity->phys->moveImpulse);
	}
	else {
		thisEntity->transform->setRotation(-thisEntity->phys->moveImpulse);
		walkingBackwards = true;
	}

	thisEntity->phys->moveImpulse *= accel;


	//if (moveDir != oldMoveDir)
	//	startTurnCycle();
}

void CPlayerC::updateViewField() {
	//update view field
	viewField.update(thisEntity->transform->hexPosition);
	//CCalcVisionField calcFieldMsg(hexPosition, viewField.ringHexes, true);
	//send(calcFieldMsg);

	THexList visibleHexes = game.level->findVisibleHexes(thisEntity->transform->hexPosition, viewField.ringHexes, true);

	std::vector<CHex> unvisibledHexes;
	for (auto hex : viewField.visibleHexes) {
		if (std::find(visibleHexes.begin(), visibleHexes.end(),
			hex) == visibleHexes.end()) {
			unvisibledHexes.push_back(hex);
		}
	}

	viewField.visibleHexes = visibleHexes;

	//update fog of war
	//CUpdateFog fogMsg(visibleHexes, unvisibledHexes);
//	send(fogMsg);

	game.level->updateVisibility(visibleHexes, unvisibledHexes);

	//hexRendr->updateFogBuffer();
	//game.level->getHexArray()->effectsNeedUpdate = true;

}

void CPlayerC::dropItem(int entityNo) {
	CItem* item = (CItem*)game.getEntity(entityNo);

	auto it = std::find(inventory.begin(), inventory.end(), item);
	inventory.erase(it);

	float dropDist = 0.75f;
	glm::vec3 dropPoint = thisEntity->getPos() + (thisEntity->transform->getUpperBodyRotationVec() * dropDist);
	item->setPosition(dropPoint);
	item->drop();
	item->parent = nullptr;
	gWin::pInv->refresh();

}

void CPlayerC::update(float dT)
{
	this->dT = dT;

	//kludge to stop sharply
	if (moveDir == moveNone)
		thisEntity->phys->velocity *= 45.0f * dT;

	updateWalkCycle();

	oldMoveDir = moveDir;
	moveDir = moveNone;
	oldWorldPos = thisEntity->transform->worldPos;


}


void CPlayerC::setMouseDir(glm::vec3& _mouseVec) {
	this->mouseVec = _mouseVec;
	thisEntity->transform->setUpperBodyRotation(mouseVec);

	if (moveDir != moveNone)
		return;

	float nearestDot = -FLT_MAX; glm::vec3 sectorNormal;
	for (int face = 0; face < 6; face++) {
		float dot = glm::dot(mouseVec, moveVector3D[face]);
		if (dot > nearestDot) {
			sectorNormal = moveVector3D[face];
			nearestDot = dot;
		}
	}

	sectorNormal = glm::normalize(sectorNormal);

	if (glm::dot(sectorNormal, thisEntity->transform->getRotationVec()) < 0.9f)
		startTurnCycle();

	thisEntity->transform->setRotation(sectorNormal);
}

void CPlayerC::addToInventory(CEntity* item) {
	CItem* takenEnt = (CItem*)item;
	takenEnt->modelCmp->setVisible(false);
	inventory.push_back(takenEnt);
	takenEnt->parent = thisEntity;
	gWin::pInv->refresh();
}

void CPlayerC::setArmour(CEntity* armour) {
	this->armour = (CArmour*)armour;
	this->armour->setParent(thisEntity);
}

void CPlayerC::setShield(CEntity* shield) {
	CItemCmp* item = (CItemCmp*)shield->item.get();
	item->setOwner(thisEntity);
	this->shield = shield;
	shieldId = shield->id;
}

void CPlayerC::startTurnCycle() {
	turningCycle = 0.125f;
}

/** Calculate where we are in the walk cycle. */
void CPlayerC::updateWalkCycle() {
	float walkMax = 1.5f;

	if (turningCycle > 0) {
		turningCycle -= dT;
		walkCycle += dT * 2;
	}
	else {
		float moveDist = glm::distance(thisEntity->getPos(), oldWorldPos);
		walkCycle += moveDist;
	}

	walkCycle = fmod(walkCycle, walkMax); //loop after travelling walkMax

	float f = walkCycle / walkMax; // 0 - 1
	f = abs(0.5f - f) * 2; //1 - 0 - 1 oscillation. 
	f = (f - 0.5f) * 2; //-1 to 1 oscillation

	footExtension = f * maxFootExtension;
	thisEntity->transform->setWalkTranslation(glm::vec3(footExtension, 0, 0));

	return;
}


void CPlayerC::setGun(CEntity* gun) {
	this->gun = (CGun*)gun;
	this->gun->setParent(thisEntity);
}

