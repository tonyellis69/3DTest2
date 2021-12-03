#pragma once

#include "item.h"

class CArmourType;

/** Generic armour entity. Create different armour types
	with plug-ins. */

class CArmour : public CItem {
public:
	int reduceDamage(int damage);
	void drop();
	std::string getShortDesc();

	std::shared_ptr<CArmourType> armourType = nullptr;


};


//!!!Armour types to plug into an armour entity;
class CArmourType {
public:
	CArmourType(CArmour* armour) {
		this->armour = armour;
	}
	virtual int reduceDamage(int damage) = 0;

	CArmour* armour;

};

class CBasicArmour : public CArmourType {
public:
	CBasicArmour(CArmour* armour) : CArmourType(armour) {};
	int reduceDamage(int damage);

	int absorption = 2;

};