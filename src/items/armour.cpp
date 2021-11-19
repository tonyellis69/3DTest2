#include "armour.h"


int CArmour::reduceDamage(int damage) {

	return armourType->reduceDamage(damage);
}



//!!!!! Different armpout types here.


int CBasicArmour::reduceDamage(int damage) {
	damage -= absorption;

	return std::max(0,damage);
}
