#include "gameHexArray.h"

void CGameHexArray::setEntityList(TEntities* pEntities) {
	entities = pEntities;
}

/** Returns true if there's an entity occupying this hex. */
bool CGameHexArray::entityCheck(CHex& hex) {
	for (auto entity : *entities) {
		//if (entity->destination == hex)
		if (entity->hexPosition == hex)
			return true;
	}
	return false;
}
