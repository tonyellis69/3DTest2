#pragma once

#include <vector>

/** Container class for hot text specific to the player's current location and situation. */
class CLocalHotList {
public:
	void clear();
	int addObject(int objId);
	int getObjectId(int localId);
	int getLocalId(int objId);


private:
	std::vector<int> objectIds;
};