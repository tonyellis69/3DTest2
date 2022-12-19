#include "gun.h"

#include "..\spawner.h"
#include "sound/sound.h"


#include <glm/gtc/matrix_transform.hpp>	

CGun::CGun() {
	entityType = entGun;
	equippable = true;
}

void CGun::fire(float firingAngle) {
	if (gunType->lastFired <= 0) {
		gunType->fire(firingAngle);
		gunType->lastFired = gunType->fireRate;
	}
}

void CGun::update(float dT) {
	CEntity::update(dT);
	if (gunType->lastFired > 0)
		gunType->lastFired -= dT;
}

void CGun::drop() {
	CItem::drop();
	CPlayerObject* player = (CPlayerObject*) parent;
	if (player->gun == this)
		player->gun = nullptr;
	parent = nullptr;
}

std::string CGun::getShortDesc() {
	if (parent) {
		CPlayerObject* player = (CPlayerObject*)parent;
		if (player->gun == this)
			return CItem::getShortDesc() + " (equipped)";
	}
	return CItem::getShortDesc();
}


std::string CGun::getMenuTextInv() {
	std::string menu = CItem::getMenuTextInv();

	if (parent) {
		CPlayerObject* player = (CPlayerObject*)parent;
		if (player->gun != this) {
			menu += "\n\\h{equip}Equip\\h";
		}
	}
	return menu;
}







//!!!Different gun types:

void CSmallGun::fire(float firingAngle) {


	auto missile = (CMissile*) spawn::missile("missile", gun->parent->getPos(), firingAngle).get();
	missile->setOwner(gun->parent);

	snd::play("shoot");
}
