#include "localHotList.h"

void CLocalHotList::clear() {
	objectIds.clear();
}

int CLocalHotList::addObject(int objId) {
	objectIds.push_back(objId);
	return objectIds.size();
}

int CLocalHotList::getObjectId(int localId) {
	if (localId > objectIds.size())
		return 0;
	return objectIds[localId-1];
}

int CLocalHotList::getLocalId(int objId) {
	for (int i = 0; i < objectIds.size(); i++) {
		if (objectIds[i] == objId)
			return i + 1;
	}
	return 0;
}
