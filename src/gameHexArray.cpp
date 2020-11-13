#include "gameHexArray.h"

#include <unordered_set>

CGameHexArray::CGameHexArray() {
	}

CGameHexArray::~CGameHexArray() {
	for (auto& entity : entities)
		delete entity;
}

void CGameHexArray::setMessageHandlers() {
	messageBus.setHandler< CGetTravelPath>(this, &CGameHexArray::onGetTravelPath);
	messageBus.setHandler< CFindActorBlock>(this, &CGameHexArray::onActorBlockCheck);
	messageBus.setHandler< CGetLineEnd>(this, &CGameHexArray::onGetLineEnd);
	messageBus.setHandler< CGetActorAt>(this, &CGameHexArray::onGetActorAt);
	messageBus.setHandler< CLineOfSight>(this, &CGameHexArray::onLineOfSight);
	messageBus.setHandler< CRandomHex>(this, &CGameHexArray::onRandomHex);
	messageBus.setHandler< CCalcVisionField>(this, &CGameHexArray::onFindViewField);
}

void CGameHexArray::setEntityList(TEntities* pEntities) {
	pEntities = pEntities;
}

/** Returns true if the transition from this hex to the next is blocked. */
bool CGameHexArray::fromToBlocked(CHex& from, CHex& to) {
	THexDir travelDirection = neighbourDirection(to, from);
	unsigned int directionBit = 1 << travelDirection;

	return getHexCube(to).blocks & directionBit;
}


//TO DO: replace with isFree below
/** Returns true if this hex is entirely free. */
bool CGameHexArray::isEmpty(glm::i32vec2& hex) {
	if (getHexOffset(hex.x, hex.y).content != 1)
		return false;

	CHex cubePos = indexToCube(hex);
	for (auto entity : entities) {
		if (entity->hexPosition == cubePos)
			return false;
	}
	return true;
}

/** Return true if there is no robot in this hex or heading into it. */
bool CGameHexArray::isFree(CHex& hex) {
	for (auto entity : entities) {
		if (entity->hexPosition == hex || entity->moveDest == hex)
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
	//entityMap.insert({ hexPos, entity });
	entity->setPosition(hexPos);
}

/** Remove this entity from the map. */
void CGameHexArray::removeFromMap(CGameHexObj* entity) {
	//for (auto entry : entityMap) {
	//	if (entry.second == entity) {
	//		entityMap.erase(entry.first);
	//		break;
	//	}
	//}

	auto removee = std::find(entities.begin(), entities.end(), entity);
	if (removee != entities.end())
		entities.erase(removee);

	auto removee2 = std::find(actors.begin(), actors.end(), entity);
	if (removee2 != actors.end())
		actors.erase(removee2);

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
				//TO DO: old hex claiming system - scrap!
				/*auto [first, last] = getEntitiesAt(indexToCube(x, y));
				for (auto it = first; it != last; it++) {
					element.blocks |= it->second->blocks();
				}*/
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
		if (hex == CHex(0, -3, 3))
			int b = 9;
	} while (unblocked && getHexCube(hex).blocks != blocksNone);

	return hex;
}

void CGameHexArray::addActor(CHexActor* actor, CHex& hex) {
	add(actor, hex);
	entities.push_back(actor);
	actors.push_back(actor);
}

void CGameHexArray::onGetTravelPath(CGetTravelPath& msg) {

	msg.travelPath = aStarPath(msg.start, msg.end, msg.fogOn);
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



void CGameHexArray::onLineOfSight(CLineOfSight& msg) {
	msg.result = lineOfSight(msg.start, msg.end);
}

void CGameHexArray::onRandomHex(CRandomHex& msg) {
	msg.hex = findRandomHex(msg.unblocked);
}

/** Return all the hexes in this view field that are visible
	from the apex.*/
void CGameHexArray::onFindViewField(CCalcVisionField& msg) {

	msg.visibleHexes = findVisibleHexes(msg.apex, *msg.perimeterHexes, msg.obsessive);
}

THexList CGameHexArray::findVisibleHexes(CHex& apex, THexList& perimeterHexes, bool obsessive) {
	std::unordered_set<CHex, hex_hash> uniqueHexes;

	int attempts;
	if (obsessive)
		attempts = 3;
	else
		attempts = 1;

	for (auto perimeterHex : perimeterHexes) {
		for (int offset = 0; offset < attempts; offset++) {
			THexList line = *hexLine4(apex, perimeterHex, offset);

			for (auto hex = line.begin() + 1; hex != line.end(); hex++) {
				uniqueHexes.insert(*hex);
				if (getHexCube(*hex).content == 2) {
					break;
				}
			}
		}
	}

	uniqueHexes.insert(apex);


	THexList visibleHexes;
	visibleHexes.assign(uniqueHexes.begin(), uniqueHexes.end());

	return visibleHexes;
}



void CGameHexArray::onFindViewField2(CCalcVisionField& msg) {
	std::unordered_set<CHex, hex_hash> uniqueHexes;


	for (auto perimeterHex : *msg.perimeterHexes) {
		THexList line = /*findLineHexes*/*hexLine(msg.apex, perimeterHex);

		for (auto hex = line.begin() + 1; hex != line.end();) {
			if (inLineOfSight2(msg.apex, (CHex&)*hex))
				uniqueHexes.insert(*hex);
			else {
				uniqueHexes.insert(*hex);
				break;
			}
			hex++;
		}
	}

	uniqueHexes.insert(msg.apex);

	msg.visibleHexes.assign(uniqueHexes.begin(), uniqueHexes.end());

}




/** Clear the fog-of-war wherever the given viewfield indicates a visible hex. */
void CGameHexArray::updateFog(THexList& visibleHexes, THexList& unvisibledHexes) {
	for (auto visibleHex : visibleHexes) {
		setFog(visibleHex, 0);
	}

	for (auto unvisibledHex : unvisibledHexes) {
		setFog(unvisibledHex, 0.5f);
	}
}

/** Return the first hex intersected by this segment. */
CHex CGameHexArray::getSegmentFirstHex(glm::vec3& A, glm::vec3& B) {
	glm::vec3 segmentDir = B - A;

	glm::vec3 segmentNormal = glm::normalize(glm::vec3(-segmentDir.y, segmentDir.x, 0));

	float smallestDist = FLT_MAX;
	CHex nearestHex;

	//starting with the hex at A...
	//for each of the surrounding hexes
	//find which has the centre closest to our segment
	CHex hexA = worldSpaceToHex(A);


	for (int dir = hexEast; dir <= hexNE; dir++) {
		CHex neighbour = getNeighbour(hexA, THexDir(dir));
		glm::vec3 centre = cubeToWorldSpace(neighbour);

		if (glm::dot(centre - A, segmentDir) < 0)
			continue;


		glm::vec3 centreToSeg = A - centre;
		float dist = abs( glm::dot(segmentNormal, centreToSeg));
		if (dist < smallestDist) {
			nearestHex = neighbour;
			smallestDist = dist;
		}
	}

	return nearestHex;
}

/** Return the direction of the neighbouring hex into which this segment exits, if any. 
	Also return the point of intersection. */
std::tuple<THexDir, glm::vec3> CGameHexArray::findSegmentExit(glm::vec3& A, glm::vec3& B, CHex& hex) {
	glm::vec3 segmentDir = B - A;
	glm::vec3 hexCentre = cubeToWorldSpace(hex);
	glm::vec3 intersection;
	THexDir exitDir = hexNone;

	for (int face = 0; face < 6; face++) {
		glm::vec3 faceA = hexCentre + corners[face];
		glm::vec3 faceB = hexCentre + corners[(face + 1) % 6];

		glm::vec3 faceDir = faceB - faceA;
		glm::vec3 facePerp = { faceDir.y,-faceDir.x , 0};

		if (glm::dot(A - faceA, facePerp) < 0) //discard faces where the segment enters the hex
			continue;

		if (segIntersect(A, B, faceA, faceB, intersection)) {
			exitDir = THexDir(face);
			break;
		}
	}
	return { exitDir, intersection };
}

