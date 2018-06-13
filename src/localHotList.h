#pragma once

#include <vector>

#include "vm.h"

/** Container class for hot text specific to the player's current location and situation. */
class CLocalHotList {
public:
	void clear();
	int addObject(int objId);
	int addObject(CObjInstance * obj);
	int getObjectId(int localId);
	int getLocalId(int objId);
	int getLocalId(CObjInstance * obj);


private:
	std::vector<int> objectIds;
};