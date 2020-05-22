#include "gameHexArray.h"

//#include "gamehextObj.h"

void CGameHexArray::setEntityList(TEntities* pEntities) {
	entities = pEntities;
}

/** Returns true if the transition from this hex to the next is blocked. */
bool CGameHexArray::fromToBlocked(CHex& from, CHex& to) {
	/*THexDir travelDirection = neighbourDirection(from, to);
	unsigned int directionBit = 1 << travelDirection;

	if (getHexCube(from).blocks & directionBit)
		return true;
	return false;*/
	//FAIL: can't use blocking info of from hex because first from hex
	//will always be entity's own hex and thus always blocked
	//all we can do is test if the destination hex is blocking our access to it.

	//find the direction of potential entry
	//can we enter the destination hex from that direction?

	THexDir travelDirection = neighbourDirection(to, from);
	unsigned int directionBit = 1 << travelDirection;

	if (getHexCube(to).blocks & directionBit)
		return true;

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

/** Move this object to a new location in the map. */
void CGameHexArray::moveEntity(CGameHexObj* entity, CHex& newHex) {
	//Remove from old position
	for (TMapIt it = entityMap.begin(); it != entityMap.end(); it++) {
		if (it->second == entity) {
			smartBlockClear((CHex&) it->first);
			entityMap.erase(it);
			break;
		}
	}

	entityMap.insert({ newHex, entity });
	getHexCube(newHex).blocks &= entity->blocks();
	
}

/** Add this entity to the map at the given position. */
void CGameHexArray::add(CGameHexObj* entity, CHex& hexPos) {
	entityMap.insert({ hexPos, entity });
	entity->setMap(this);
	entity->setPosition(hexPos);
}

/** Return all entities at this hex position. */
TRange CGameHexArray::getEntitiesAt(CHex& hex) {
	return entityMap.equal_range(hex);
}

/** Return the first entity at this hex position. */
CGameHexObj* CGameHexArray::getEntityAt(CHex& hex) {
	auto [first, last] = getEntitiesAt(hex);
	for (auto it = first; it != last; it++) {
			return it->second;
	}

	return NULL;;
}

CGameHexObj* CGameHexArray::getEntityClassAt(int classId, CHex& hex) {
	auto [first, last] = getEntitiesAt(hex);
	for (auto it = first; it != last; it++) {
		if (it->second->isTigClass(classId))
		return it->second;
	}
	return NULL;
}


CGameHexObj* CGameHexArray::getBlockingEntityAt(CHex& hex) {
	auto [first, last] = getEntitiesAt(hex);
	for (auto it = first; it != last; it++) {
		if (it->second->blocks())
			return it->second;
	}

	return NULL;
}

CGameHexObj* CGameHexArray::getEntityNotSelf(CGameHexObj* self) {
	auto [first, last] = getEntitiesAt(self->hexPosition);
	for (auto it = first; it != last; it++) {
		if (it->second == self)
			continue;
		return it->second;
	}

	return NULL;
}

/** Refresh the map's blocking information. */
void CGameHexArray::updateBlocking() {
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			CHexElement& element = getHexOffset(x, y);
			element.blocks = blocksNone;
			if (element.content == 2)
				element.blocks = blocksAll;
			else {
				auto [first, last] = getEntitiesAt(indexToCube(x, y));
				for (auto it = first; it != last; it++) {
					element.blocks |= it->second->blocks();
				}
			}
		}
	}
	
	//ensure doors can't be exited from the wrong angle
	for (auto keyValue : entityMap) {
		CGameHexObj& entity = *keyValue.second;
		if (entity.isTigClass(tig::CDoor)) {
			unsigned int blockEffect = entity.blocks();
			for (unsigned int dir = hexEast; dir < hexNone; dir++) {
				if (blockEffect & (1 << dir)) {
					CHex neighbour = getNeighbour(entity.hexPosition, THexDir(dir));
					unsigned int oppositeDir = opposite(THexDir(dir));
					getHexCube(neighbour).blocks |= (1 << oppositeDir);
				}
			}
		}
	}
}

/** Clear this hex of any current blocks, but restore those caused by
	neighbours or occupying entities. */
void CGameHexArray::smartBlockClear( CHex& pos) {
	getHexCube(pos).blocks = blocksNone;

	auto [first, last] = getEntitiesAt(pos);
	for (auto it = first; it != last; it++) {
		getHexCube(pos).blocks |= first->second->blocks();
	}

	for (unsigned int dir = hexEast; dir < hexNone; dir++) {
		CHex neighbour = getNeighbour(pos, THexDir(dir));
		auto [first, last] = getEntitiesAt(neighbour);
		for (auto it = first; it != last; it++) {
			if (it->second->isTigClass(tig::CDoor) && it->second->blocks(THexDir(dir))) {
				getHexCube(pos).blocks |= (1 << dir);
			}
		}
	}
}
