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



CPlayerObject::CPlayerObject() {

	viewField.centre = CHex(0);
	viewField.setField(10);

	msg::attach(msg::tmpMsg, this, &CPlayerObject::tmpKeyCB); //temp!

	physics.invMass = 1.0f/80.0f; //temp!

}

CPlayerObject::~CPlayerObject() {

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
		world.map->addExistingEntity( item, hexPosition );
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

		world.map->removeEntity(item);
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
	//auto missile = std::dynamic_pointer_cast<CMissile> (spawn::missile("missile", worldPos, targetAngle));
	auto missile = spawn::missile("missile", worldPos, targetAngle);
	missile->setOwner(this);
	//missile->setSpeed(7.0f);

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
		world.map->removeEntity(this);
		dead = true;
		visible = false;
		world.onPlayerDeath();
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

	THexList visibleHexes = world.map->findVisibleHexes(hexPosition, viewField.ringHexes, true);

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

	world.map->updateVisibility(visibleHexes, unvisibledHexes);

	//hexRendr->updateFogBuffer();
	world.map->effectsNeedUpdate = true;

}

/** Respond to player move instruction. */
void CPlayerObject::moveCommand(TMoveDir commandDir) {
	if (dead)
		return;
	const float accel = 3000;
	switch (commandDir) {
	case moveNorth:  physics.moveImpulse = { 0,1,0 }; break;
	case moveNE: physics.moveImpulse = { M_SQRT1_2, M_SQRT1_2, 0 }; break;
	case moveEast: physics.moveImpulse = { 1,0,0 }; break;
	case moveSE: physics.moveImpulse = { M_SQRT1_2, -M_SQRT1_2, 0 }; break;
	case moveSouth: physics.moveImpulse = { 0,-1,0 }; break;
	case moveSW: physics.moveImpulse = { -M_SQRT1_2, -M_SQRT1_2, 0 }; break;
	case moveWest: physics.moveImpulse = { -1,0,0 }; break;
	case moveNW: physics.moveImpulse = { -M_SQRT1_2, M_SQRT1_2, 0 }; break;
	}

	//turn to that direction
	rotation = glm::orientedAngle(glm::normalize(physics.moveImpulse), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

	physics.moveImpulse *= accel;
}


void CPlayerObject::update(float dT) {
	this->dT = dT;

	if (!visible && visibilityCooldown < 3.0f) {
		visibilityCooldown += dT;
		if (visibilityCooldown > 3.0f)
			visible = true;
	}
}

void CPlayerObject::setTargetAngle(float angle) {
	targetAngle = angle;
}


/** Check if the given segment intersects us. */
std::tuple<bool, glm::vec3> CPlayerObject::collisionCheck(glm::vec3& segA, glm::vec3& segB) {
	if (lineModel.BBcollision(segA, segB))
		return { true, glm::vec3() };

	return { false, glm::vec3() };
}





