#include "armour.h"

//#include "..\playerHexObj.h"

CArmour::CArmour() {
	equippable = true;
}

int CArmour::reduceDamage(int damage) {

	return armourType->reduceDamage(damage);
}

void CArmour::drop() {
	CItem::drop();
	CEntity* player = parent;
	if (player->playerC->armour == this)
		player->playerC->armour = nullptr;
	parent = nullptr;
}


std::string CArmour::getShortDesc() {
	if (parent) {
		CEntity* player = parent;
		if (player->playerC->armour == this)
			return CItem::getShortDesc() + " (worn)";
	}
	return CItem::getShortDesc();
}

std::string CArmour::getMenuTextInv() {
	std::string menu = CItem::getMenuTextInv();

	if (parent) {
		CEntity* player = parent;
		if (player->playerC->armour != this) {
			menu += "\n\\h{equip}Equip\\h";
		}
	}
	return menu;
}


//!!!!! Different armout types here.


int CBasicArmour::reduceDamage(int damage) {
	damage -= absorption;

	return std::max(0,damage);
}
