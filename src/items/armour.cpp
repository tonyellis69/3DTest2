#include "armour.h"

#include "..\playerHexObj.h"

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


//!!!!! Different armpout types here.


int CBasicArmour::reduceDamage(int damage) {
	damage -= absorption;

	return std::max(0,damage);
}
