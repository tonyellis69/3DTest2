#include "playerHexObj.h"

#include <cmath>

#include <glm/gtx/vector_angle.hpp> 

#include "utils/log.h"

//#include "hexItem.h"

#include "gameState.h"

#include "missile.h"

#include "sound/sound.h"

#include "messaging/msg2.h"

#include "gameGui.h"

#include "spawner.h"

#include "renderer/imRendr/imRendr.h"

constexpr float sin30 = 0.5f;
constexpr float sin60 = 0.86602540378443;

CPlayerObject::CPlayerObject() {

	viewField.centre = CHex(0);
	viewField.setField(10);

	msg::attach(msg::tmpMsg, this, &CPlayerObject::tmpKeyCB); //temp!

	physics.invMass = 1.0f/80.0f; //temp!

	//setBoundingRadius(); //temp?
	entityType = entPlayer;
}

CPlayerObject::~CPlayerObject() {

}

void CPlayerObject::setModel(TModelData& model) {
	lineModel.model = model;
	upperBody = lineModel.getNode("body");
	leftFoot = lineModel.getNode("footL");
	rightFoot = lineModel.getNode("footR");
	setBoundingRadius();
}

void CPlayerObject::buildWorldMatrix() {
	glm::mat4 worldM = glm::translate(glm::mat4(1), worldPos);

	upperBody->matrix = worldM;
	upperBody->matrix = glm::rotate(worldM, upperBodyRotation, glm::vec3(0, 0, -1));


	worldM = glm::rotate(worldM, rotation, glm::vec3(0, 0, -1));

	//leftFoot->matrix = worldM;
	//rightFoot->matrix = worldM;

	rightFoot->matrix = glm::translate(worldM, glm::vec3(footExtension, 0, 0));;
	leftFoot->matrix = glm::translate(worldM, glm::vec3(-footExtension,0,0));


}

/** Temporary keystroke catcher. */
void CPlayerObject::tmpKeyCB(int key) {

	if (key >= '1' && key <= '5' && inventoryOn) {
		itemSelected = key - '1';
		auto item = inventory[itemSelected];
		std::string desc = "\n\n" + item->itemType + "\nU) use or (L) leave?";
		gWin::addText("con", desc);
		return;
	}

	if (key == 'U' && inventoryOn) {
		gWin::addText("con", "\nUsed!");
		return;
	}

	if (key == 'L' && inventoryOn) {
		gWin::addText("con", "\nDropped!");
		auto item = inventory[itemSelected];
		inventory.erase(inventory.begin() + itemSelected);
		game.map->addExistingEntity( item, hexPosition );
		itemSelected = -1;
		return;
	}



	if (key >= '1' && key <= '5' && !tmpFloorItems.empty()) {
		itemSelected = key - '1';
		auto item = tmpFloorItems[itemSelected];
		std::string desc = "\n\n" + item->itemType + "\n";
		desc += item->description;
		desc += "\n\nT) Take?";
		gWin::addText("con",desc);
	}

	if (key == 'T' && itemSelected != -1) {
		auto item = tmpFloorItems[itemSelected];
		inventory.push_back(item);

		game.map->removeEntity(item);
	}

	if (key == 'I') {
		gWin::addText("con", "\nInventory:\n");
		if (inventory.empty()) {
			gWin::addText("con", "Nothing!");
			return;
		}

		std::string invTxt;
		auto itemNo = '1';
		for (auto& item : inventory) {
			invTxt = itemNo;
			invTxt += ") " + item->itemType + "\n";
			itemNo++;
			gWin::addText("con", invTxt);
		}
		inventoryOn = true;


	}

}

/** Player has pressed or released the fire button. */
void CPlayerObject::onFireKey(bool pressed) {
	if (!pressed || dead)
		return;

	//hard-coded default action: launch a missile!
	float targetAngle = getUpperBodyRotation();
	auto missile = spawn::missile("missile", worldPos, targetAngle);
	missile->setOwner(this);

	snd::play("shoot");
}




void CPlayerObject::draw() {
	//for (auto hex : viewField.visibleHexes)
	//	hexRendr->highlightHex(hex);
	if (dead)
		return;
	CEntity::draw();
}





void CPlayerObject::receiveDamage(CEntity& attacker, int damage) {
	liveLog << "\nPlayer hit!";

	hp--;

	if (hp < 1) {
		game.map->removeEntity(this);
		dead = true;
		visible = false;
		game.onPlayerDeath();
	}
}



/** Called when player has arrived at a new hex.*/
void CPlayerObject::onMovedHex() {
	CActorMovedHex msg(hexPosition, this);
	//send(msg);
	//eventually calls hexWorld alertEntitiesInPlayerFov, which notifies entities
	//if they're now in view of the player. Find a more direct way to do this

	updateViewField();

	gWin::clearText("con");
	tmpFloorItems.clear();
	itemSelected = -1;
	inventoryOn = false;
	
	//auto [first, last] = world.map->getEntitiesAt(hexPosition);
	//for (auto& it = first; it != last; it++) {
	//	if (it->second->isItem) {
	//		tmpFloorItems.push_back((CItem*)(it->second));
	//	}
	//}
	//!!!!TO DO: will need replacement

	if (!tmpFloorItems.empty()) {
		auto menuChar = '1';
		std::string stuff = "You can see:";
		for (auto& item : tmpFloorItems) {
			stuff += "\n"; stuff += menuChar;
			stuff += ") A " + item->itemType;
			menuChar++;
		}
		gWin::addText("con", stuff);
	}

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


	const float accel = 3000;
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

/** Check if the given segment intersects us. */
std::tuple<bool, glm::vec3> CPlayerObject::collisionCheck(glm::vec3& segA, glm::vec3& segB) {
	if (lineModel.BBcollision(segA, segB))
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




	//float taper;

	//if (turningCycle > 0) {
	//	turningCycle -= dT;
	//	taper = 1.0f;
	//}
	//else
	//	taper= glm::smoothstep(0.0f, 2.0f, glm::length(physics.velocity));

	//walkCycle += dT * 10.0f * taper;
	//walkCycle = fmod(walkCycle, 2*M_PI); 

	//footExtension = sin(walkCycle) * maxFootExtension;

}




