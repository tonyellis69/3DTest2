#include "gameHexArray.h"

#include <unordered_set>
#include <algorithm>

#include "utils/log.h"
#include "utils/random.h"


#include "utils/files.h"

#include "robot.h"

#include "spawner.h"



//!!!!!!!!!!!!!!!!!!!
//! temporary fix - will need doing better
CEntity* CGameHexArray::getEntityAt2(const CHex& hex) {
	for (auto& entity : entities) {
		if (entity->hexPosition == hex)
			return entity.get();
	}

	return nullptr;
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


/** Clear the fog-of-war wherever the given viewfield indicates a now-visible hex. 
	Set visibility to zero where a hex has now become non-visible. */
void CGameHexArray::updateVisibility(THexList& visibleHexes, THexList& unvisibledHexes) {
	for (auto& visibleHex : visibleHexes) {
		setFog(visibleHex, 0);
		setVisibility(visibleHex, 1.0f);
	}

	for (auto& unvisibledHex : unvisibledHexes) {
		setVisibility(unvisibledHex, 0.5f);
	}
}


/** Register this entity as being on the map at the given hex.*/
void CGameHexArray::addEntity(TEntity entity, CHex& hex) {
	entity->setPosition(hex);
	entities.push_back(entity);
}

void CGameHexArray::addExistingEntity(CEntity* entity, CHex& hex)
{
	entity->setPosition(hex);

}


void CGameHexArray::removeEntity(CEntity* entity) {
	//for (auto& it = entityMap.begin(); it != entityMap.end(); it++) {
	//	if (it->second == entity) {
	//		it = entityMap.erase(it);
	//		break;
	//	}
	//}
	entity->setPosition(CHex(-1));
}




/** Tag an entity for immediate deletion from the map. */
void CGameHexArray::deleteEntity(CEntity& entity) {
	entity.deleteMe = true;
	entityListDirty = true;
}

/** Remove any enities marked for deletion from the entity lists. */
void CGameHexArray::tidyEntityLists() {
	if (entityListDirty) {
		

		for (auto& it = entities.begin(); it != entities.end(); ) {
			if (it->get()->deleteMe)
				it = entities.erase(it);
			else
				it++;
		}

		entityListDirty = false;
	}
}

void CGameHexArray::save(std::ostream& out) {
	int version = 1;
	file::writeObject(version,out);
	
	CMapHeader header = {width, height};
	file::writeObject(header,out);
	for (auto& hex :  flatArray) {
		file::writeObject(hex,out);
	}

	int numBots = std::count_if(entities.begin(), entities.end(),
		[](TEntity entity) {return entity->isRobot; });
	file::writeObject(numBots, out);
	for (auto& ent : entities) {
		if (ent->isRobot)
			file::writeObject(ent->worldPos, out);
	}
}

void CGameHexArray::load(std::istream& in) {
	int version;
	file::readObject(version, in);

	CMapHeader header;
	file::readObject(header, in);

	int numHexes = header.height * header.width;
	TFlatArray flatArray(numHexes);
	for (auto& hex : flatArray) {
		file::readObject(hex, in);
	}
	setArray(flatArray);

	int numBots;
	file::readObject(numBots, in);
	for (int n = 0; n < numBots; n++) {
		glm::vec3 pos;
		file::readObject(pos, in);
		spawn::robot("robot", pos);
	}
}


