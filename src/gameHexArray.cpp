#include "gameHexArray.h"

//#include "gamehextObj.h"

void CGameHexArray::setEntityList(TEntities* pEntities) {
	entities = pEntities;
}

/** Returns true if there's an entity occupying this hex. */
bool CGameHexArray::entityCheck(CHex& hex) {
	for (auto entity : *entities) {
		if (entity->blocks && entity->hexPosition == hex)
			return true;
	}
	return false;
}

CHex CGameHexArray::findLineEnd(CHex& start, CHex& target) {
	return CHexArray::findLineEnd(start, target);
}
