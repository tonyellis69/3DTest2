#include "gameHexArray.h"

#include <unordered_set>

CGameHexArray::CGameHexArray() {
	}

void CGameHexArray::setMessageHandlers() {
	messageBus.setHandler< CGetTravelPath>(this, &CGameHexArray::onGetTravelPath);
	messageBus.setHandler< CMoveEntity>(this, &CGameHexArray::onMoveEntity);
	messageBus.setHandler< CFindActorBlock>(this, &CGameHexArray::onActorBlockCheck);
	messageBus.setHandler< CGetLineEnd>(this, &CGameHexArray::onGetLineEnd);
	messageBus.setHandler< CGetActorAt>(this, &CGameHexArray::onGetActorAt);
	messageBus.setHandler< CLineOfSight>(this, &CGameHexArray::onLineOfSight);
	messageBus.setHandler< CRandomHex>(this, &CGameHexArray::onRandomHex);
	messageBus.setHandler< CFindVisionField>(this, &CGameHexArray::onFindViewField);
}

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
	entity->setPosition(hexPos);
}

/** Remove this entity from the map. */
void CGameHexArray::removeFromEntityList(CGameHexObj* entity) {
	for (auto entry : entityMap) {
		if (entry.second == entity) {
			entityMap.erase(entry.first);
			return;
		}
	}
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

CHexActor* CGameHexArray::getRobotAt(CHex& hex) {
	return (CHexActor*)getEntityClassAt(tig::CRobot2, hex);
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

/** Returns true if there is an unblocked straight line of hexes
	between these points. */
bool CGameHexArray::lineOfSight(CHex& start, CHex& end) {
	CHex stopHex = findLineEnd(start, end);
	if (stopHex == end)
		return true;
	return false;
}

/** Return a random hex somewhere on the map. */
CHex CGameHexArray::findRandomHex(bool unblocked) {
	CHex hex;
	do {
		CDiceRoll msg(width, height);
		send(msg);
		hex = indexToCube(msg.result, msg.result2);
	} while (unblocked && !getHexCube(hex).blocks == blocksNone);

	return hex;
}

void CGameHexArray::onGetTravelPath(CGetTravelPath& msg) {

	int b = getHexCube(CHex(0, 4, -4)).blocks;
	//zero here!!!

	msg.travelPath = aStarPath(msg.start, msg.end);
}

void CGameHexArray::onMoveEntity(CMoveEntity& msg) {
	moveEntity(msg.entity, msg.newHex);

}

/** If an actor is blocking this hex, return it. */
void CGameHexArray::onActorBlockCheck(CFindActorBlock& msg) {
	auto [first, last] = getEntitiesAt(msg.hex);
	for (auto it = first; it != last; it++) {
		if (it->second->isActor()) {
			msg.blockingActor = (CHexActor*)it->second;
			return;
		}
	}
}

void CGameHexArray::onGetLineEnd(CGetLineEnd& msg) {
	msg.end = findLineEnd(msg.start, msg.end);
}

void CGameHexArray::onGetActorAt(CGetActorAt& msg) {
	auto [first, last] = getEntitiesAt(msg.hex);
	for (auto it = first; it != last; it++) {
		if (it->second->isActor() &&  it->second != msg.notActor) {
			msg.actor = (CHexActor*)it->second;
			return;
		}
	}
}

void CGameHexArray::onGetObjectAt(CGetObjectAt& msg) {
	msg.obj = getEntityAt(msg.hex);
}

void CGameHexArray::onLineOfSight(CLineOfSight& msg) {
	msg.result = lineOfSight(msg.start, msg.end);
}

void CGameHexArray::onRandomHex(CRandomHex& msg) {
	msg.hex = findRandomHex(msg.unblocked);
}

/** Return all the hexes in this view field that are visible
	from the apex.*/
void CGameHexArray::onFindViewField(CFindVisionField& msg) {
	std::unordered_set<CHex, hex_hash> uniqueHexes;

	for (auto arcHex : *msg.arc) {
		THexList line = /*findLineHexes*/*hexLine(msg.apex, arcHex);
		uniqueHexes.insert(line.begin() + 1, line.end());
	}

	for (auto hex = uniqueHexes.begin(); hex != uniqueHexes.end();) {
		if (lineOfSight2(msg.apex,(CHex&)*hex) == false)
			 hex = uniqueHexes.erase(hex);
		else
			hex++;
	}



	msg.visibleHexes.assign(uniqueHexes.begin(), uniqueHexes.end());

}
