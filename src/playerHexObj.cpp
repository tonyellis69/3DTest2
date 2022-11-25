#include "playerHexObj.h"

#include <cmath>

#include <glm/gtx/vector_angle.hpp> 

#include "utils/log.h"

#include "gameState.h"

#include "missile.h"

#include "sound/sound.h"

#include "gameGui.h"

#include "spawner.h"

#include "..\3Dtest\src\hexRenderer.h"

#include "gameWin.h"

#include "hexRender/multiDraw.h"

#include "listen/listen.h"

#include "gameEvent.h"

#include "items/shield.h"

constexpr float sin30 = 0.5f;
constexpr float sin60 = 0.86602540378443;

CPlayerObject::CPlayerObject() {

	viewField.centre = CHex(0);
	viewField.setField(10);


	physics.invMass = 1.0f/80.0f; //temp!

	//setBoundingRadius(); //temp?
	entityType = entPlayer;

}

CPlayerObject::~CPlayerObject() {

}



void CPlayerObject::setModel(CModel& modelRef) {
	this->model = modelRef;
	upperBody = model.getMesh("body");
	leftFoot = model.getMesh("footL");
	rightFoot = model.getMesh("footR");
	upperBodyMask = model.getMesh("body_mask");
	leftFootMask = model.getMesh("footL_mask");
	rightFootMask = model.getMesh("footR_mask");

	this->model.setMainMesh("body");

	setBoundingRadius();
}

void CPlayerObject::buildWorldMatrix() {
	glm::mat4 worldM = glm::translate(glm::mat4(1), worldPos);

	upperBody->matrix = worldM;
	upperBody->matrix = glm::rotate(worldM, upperBodyRotation, glm::vec3(0, 0, -1));

	//lineModel.model.matrix = upperBody->matrix;
	//model.tmpMatrix = upperBody->matrix;
	//FIXME! Temp kludge to ensure collision check works
	//maybe solve by giving every model a collision subModel to check against.
	upperBodyMask->matrix = upperBody->matrix;

	worldM = glm::rotate(worldM, rotation, glm::vec3(0, 0, -1));

	rightFoot->matrix = glm::translate(worldM, glm::vec3(footExtension, 0, 0));
	leftFoot->matrix = glm::translate(worldM, glm::vec3(-footExtension,0,0));

	leftFootMask->matrix = leftFoot->matrix;
	rightFootMask->matrix = rightFoot->matrix;

}

void CPlayerObject::initDrawFn() {
	drawFn = std::make_shared<CMultiDraw>(this);
	auto fn = (CMultiDraw*)drawFn.get();
	fn->lowerMeshes.push_back(model.getMesh("footL"));
	fn->lowerMeshes.push_back(model.getMesh("footR"));
	fn->upperMeshes.push_back(model.getMesh("body"));
	fn->upperMask = model.getMesh("body_mask");
	fn->lowerMasks.push_back(model.getMesh("footL_mask"));
	fn->lowerMasks.push_back(model.getMesh("footR_mask"));
}



void CPlayerObject::dropItem(int entityNo) {
	CItem* item = (CItem*)game.map->getEntity(entityNo);

	auto it = std::find(inventory.begin(), inventory.end(), item);
	inventory.erase(it);

	float dropDist = 0.75f;
	glm::vec3 dropPoint = worldPos + (getUpperBodyRotationVec() * dropDist);
	item->setPosition(dropPoint);
	item->drop();
	item->parent = nullptr;
	gWin::pInv->refresh();

}






/** Player has pressed or released the fire button. */
void CPlayerObject::onFireKey(bool pressed) {
	if (!pressed || dead || gun == nullptr)
		return;

	gun->fire(getUpperBodyRotation());
}




void CPlayerObject::draw() {
	//for (auto hex : viewField.visibleHexes)
	//	hexRendr->highlightHex(hex);
	if (dead)
		return;
	//CEntity::draw();
	//hexRendr2.drawLineModel(*leftFoot);
	//hexRendr2.drawLineModel(*rightFoot);
	//hexRendr2.drawLineModel(*upperBody);
	//hexRendr2.drawLineModel(model.meshes[2]);
	//hexRendr2.drawLineModel(model.meshes[1]);
	//hexRendr2.drawLineModel(model.meshes[0]);
}





void CPlayerObject::receiveDamage(CEntity& attacker, int damage) {
	if (shield) {
		CShieldComponent* playerShield = (CShieldComponent*) shield->item.get();
		damage = playerShield->absorbDamage(damage);
		if (damage <= 0)
			return;
	}




	liveLog << "\nPlayer hit!";

	int finalDamage = damage;// armour->reduceDamage(damage);

	hp -= finalDamage;

	if (hp < 1) {
		//game.map->removeEntity(this);
		dead = true;
		visible = false;
		//game.onPlayerDeath();
		

		CGameEvent e;
		lis::event(e);

	}
}



/** Called when player has arrived at a new hex.*/
void CPlayerObject::onMovedHex() {
	//CActorMovedHex msg(hexPosition, this);
	//send(msg);
	//eventually calls hexWorld alertEntitiesInPlayerFov, which notifies entities
	//if they're now in view of the player. Find a more direct way to do this

	//updateViewField();
	//turned off 30/8/22 - needed?

	//gWin::clearText("con");
	//tmpFloorItems.clear();
	itemSelected = -1;
	inventoryOn = false;
	


}


void CPlayerObject::updateViewField() {
	//update view field
	viewField.update(hexPosition);
	//CCalcVisionField calcFieldMsg(hexPosition, viewField.ringHexes, true);
	//send(calcFieldMsg);

	THexList visibleHexes = game.map->findVisibleHexes(hexPosition, viewField.ringHexes, true);

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

	game.map->updateVisibility(visibleHexes, unvisibledHexes);

	//hexRendr->updateFogBuffer();
	game.map->getHexArray()->effectsNeedUpdate = true;

}

/** Respond to player move instruction. */
void CPlayerObject::moveCommand(TMoveDir commandDir) {
	if (dead)
		return;


	const float accel = 4000;
	switch (commandDir) {
	case moveNorth:  physics.moveImpulse = { 0,1,0 }; break;
	case moveNE: physics.moveImpulse = { sin30, sin60, 0 }; break;
	case moveEast: physics.moveImpulse = { 1,0,0 }; break;
	case moveSE: physics.moveImpulse = { sin30, -sin60, 0 }; break;
	case moveSouth: physics.moveImpulse = { 0,-1,0 }; break;
	case moveSW: physics.moveImpulse = { -sin30, -sin60, 0 }; break;
	case moveWest: physics.moveImpulse = { -1,0,0 }; break;
	case moveNW: physics.moveImpulse = { -sin30, sin60, 0 }; break;
	}


	moveDir = commandDir;


	//turn to that direction
	walkingBackwards = false;
	glm::vec3 upperRotation = getUpperBodyRotationVec();
	if (glm::dot(physics.moveImpulse, upperRotation) >= 0) {
		setRotation(physics.moveImpulse);
	}
	else {
		setRotation(-physics.moveImpulse);
		walkingBackwards = true;
	}

	physics.moveImpulse *= accel;


	//if (moveDir != oldMoveDir)
	//	startTurnCycle();
}


void CPlayerObject::update(float dT) {
	this->dT = dT;

	if (!visible && visibilityCooldown < 3.0f) {
		visibilityCooldown += dT;
		if (visibilityCooldown > 3.0f)
			visible = true;
	}

	//kludge to stop sharply
	if (moveDir == moveNone)
		physics.velocity *= 45.0f *dT;

	updateWalkCycle();

	buildWorldMatrix();
	oldMoveDir = moveDir;
	moveDir = moveNone;

}

void CPlayerObject::setTargetAngle(float angle) {
	//targetAngle = angle;

}

void CPlayerObject::setRotation(glm::vec3& vec) {
	rotation = glm::orientedAngle(vec, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

}

glm::vec3 CPlayerObject::getRotation() {
	return { cos(rotation), -sin(rotation),0 };
}

void CPlayerObject::setUpperBodyRotation(float angle) {
	upperBodyRotation = angle;

}

void CPlayerObject::setUpperBodyRotation(glm::vec3& vec) {
	upperBodyRotation = glm::orientedAngle(vec, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
}

float CPlayerObject::getUpperBodyRotation() {
	return upperBodyRotation;
}

glm::vec3 CPlayerObject::getUpperBodyRotationVec() {
	return  { cos(upperBodyRotation), -sin(upperBodyRotation),0 };
}

void CPlayerObject::setMouseDir(glm::vec3& mouseVec) {
	this->mouseVec = mouseVec;
	setUpperBodyRotation(mouseVec);

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

	if (glm::dot(sectorNormal,getRotation()) < 0.9f)
		startTurnCycle();

	setRotation(sectorNormal);
}

void CPlayerObject::addToInventory(CEntity* item) {
	CItem* takenEnt = (CItem*)item;
	inventory.push_back(takenEnt);
	takenEnt->parent = this;
	gWin::pInv->refresh();
}

void CPlayerObject::setGun(CEntity* gun) {
	this->gun = (CGun*) gun;
	this->gun->setParent(this);
}

void CPlayerObject::setArmour(CEntity* armour) {
	this->armour = (CArmour*) armour;
	this->armour->setParent(this);
}

/** Check if the given segment intersects us. */
std::tuple<bool, glm::vec3> CPlayerObject::collisionCheck(glm::vec3& segA, glm::vec3& segB) {
	if (model.BBcollision(segA, segB))
		return { true, glm::vec3() };

	return { false, glm::vec3() };
}

void CPlayerObject::startTurnCycle() {
	turningCycle = 0.125f;
}

/** Calculate where we are in the walk cycle. */
void CPlayerObject::updateWalkCycle() {
	float walkMax = 1.5f;

	if (turningCycle > 0) {
		turningCycle -= dT;
		walkCycle += dT * 2;
	}
	else {
		float moveDist = glm::distance(worldPos, oldWorldPos);
		walkCycle += moveDist;
	}

	walkCycle =  fmod(walkCycle, walkMax); //loop after travelling walkMax

	float f = walkCycle / walkMax; // 0 - 1
	f = abs(0.5f - f) * 2; //1 - 0 - 1 oscillation. 
	f = (f - 0.5f) * 2; //-1 to 1 oscillation

	footExtension = f * maxFootExtension;

	return;
}





