#include "gameHexArray.h"

//#include "gamehextObj.h"

void CGameHexArray::setEntityList(TEntities* pEntities) {
	entities = pEntities;
}

/** Returns true if there's an entity occupying this hex. */
bool CGameHexArray::entityBlockCheck(CHex& current, CHex& hex) {
	for (auto entity : *entities) {
		if (entity->blocks && entity->hexPosition == hex)
			return true;

		//are we trying to get somewhere from a door?
		if (entity->hexPosition == current && entity->isTigClass(tig::CDoor)) {
			THexDir angle = neighbourDirection(current, hex);
			if (angle != entity->getDirection() && opposite(angle) != entity->getDirection())
				return true;

		}

		//are we trying to get somewhere through a door?
		if (entity->hexPosition == hex && entity->isTigClass(tig::CDoor)) {
			THexDir angle = neighbourDirection(current, hex);
			if (angle != entity->getDirection() && opposite(angle) != entity->getDirection())
				return true;

		}
	}
	return false;
}

/** Returns true if this hex is entirely freee. */
bool CGameHexArray::isEmpty(glm::i32vec2& hex) {
	if (getHexOffset(hex.x, hex.y).content != 1)
		return false;

	CHex cubePos = indexToCube(hex);
	for (auto entity : *entities) {
		if (entity->hexPosition == cubePos)
			return false;
	}
	return true;
}

CHex CGameHexArray::findLineEnd(CHex& start, CHex& target) {
	return CHexArray::findLineEnd(start, target);
}
