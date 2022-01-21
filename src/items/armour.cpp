#include "armour.h"

#include "..\playerHexObj.h"

CArmour::CArmour() {
	equippable = true;
}

int CArmour::reduceDamage(int damage) {

	return armourType->reduceDamage(damage);
}

void CArmour::drop() {
	CPlayerObject* player = (CPlayerObject*)parent;
	if (player->armour == this)
		player->armour = nullptr;
	parent = nullptr;
}


std::string CArmour::getShortDesc() {
	if (parent) {
		CPlayerObject* player = (CPlayerObject*)parent;
		if (player->armour == this)
			return CItem::getShortDesc() + " (worn)";
	}
	return CItem::getShortDesc();
}

std::string CArmour::getMenuTextInv() {
	std::string menu = CItem::getMenuTextInv();

	if (parent) {
		CPlayerObject* player = (CPlayerObject*)parent;
		if (player->armour != this) {
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
