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
CEntity* CMap::getEntityAt2(const CHex& hex) {
	for (auto& entity : entities) {
		if (entity->transform->hexPosition == hex)
			return entity.get();
	}

	return nullptr;
}

//!!!!!!!!!!!!!!!!
//Temporary fix - consider map etc for speed, quadtree
CEntities CMap::getEntitiesAt(const CHex& hex) {
	CEntities hexEntities;
	for (auto& entity : entities) {
		if (entity->transform->hexPosition == hex)
			hexEntities.push_back(entity.get());
	}

	return hexEntities;
}


THexList CMap::findVisibleHexes(CHex& apex, THexList& perimeterHexes, bool obsessive) {
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
				if (hexArray.getHexCube(*hex).content == 2) {
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
void CMap::updateVisibility(THexList& visibleHexes, THexList& unvisibledHexes) {
	for (auto& visibleHex : visibleHexes) {
		//hexArray.setFog(visibleHex, 0);
		hexArray.setVisibility(visibleHex, 1.0f);
	}

	for (auto& unvisibledHex : unvisibledHexes) {
		hexArray.setVisibility(unvisibledHex, 0.5f);
	}
}


///** Register this entity as being on the map at the given hex.*/
//void CMap::addEntity(TEntity entity, CHex& hex) {
//	entity->setPosition(hex);
//	entities.push_back(entity);
//}

/** Basic add-to-world without specifying a location. For items in
	posession of another entity, not yet in play, etc.*/
void CMap::addEntity(TEntity entity) {
	entities.push_back(entity);
}

//void CMap::addExistingEntity(CEntity* entity, CHex& hex)
//{
//	entity->setPosition(hex);
//
//}


//void CMap::removeEntity(CEntity* entity) {
//
//	//entity->setPosition(CHex(-1));
//}

void CMap::removeEntity(TEntity entity) {
	entity->deleteMe = true;
	entitiesToDelete = true;
}




/** Tag an entity for immediate deletion from the map. */
void CMap::deleteEntity(CEntity& entity) {
	entity.deleteMe = true;
	entitiesToDelete = true;
}

/** Remove any enities marked for deletion from the entity lists. */
void CMap::tidyEntityLists() {
	if (entitiesToDelete) {
		

		for (auto& it = entities.begin(); it != entities.end(); ) {
			if (it->get()->deleteMe)
				it = entities.erase(it);
			else
				it++;
		}

		entitiesToDelete = false;
	}
}

/** Save the map details to file. */
void CMap::save(std::ostream& out) {
	int version = 1;
	file::writeObject(version,out);
	
	CMapHeader header = {hexArray.width, hexArray.height};
	file::writeObject(header,out);
	for (auto& hex :  hexArray.getFlatArray()) {
		file::writeObject(hex,out);
	}

	int numEnts = entities.size();
	file::writeObject(numEnts, out);
	for (auto& ent : entities) {
		file::writeObject(ent->entityType, out);
		file::writeObject(ent->getPos(), out);
	}
}

void CMap::load(std::istream& in) {
	int version;
	file::readObject(version, in);

	CMapHeader header;
	file::readObject(header, in);

	int numHexes = header.height * header.width;
	TFlatArray flatArray(numHexes);
	for (auto& hex : flatArray) {
		file::readObject(hex, in);
	}
	hexArray.setArray(flatArray);

	entities.clear();

	int numEnts;
	file::readObject(numEnts, in);
	for (int n = 0; n < numEnts; n++) {
		TEntityType entType;
		file::readObject(entType, in);

		glm::vec3 pos;
		file::readObject(pos, in);

		switch (entType) {
		case entPlayer: spawn::player("player", pos); break;
		case entMeleeBot: spawn::robot("melee bot", pos); break;
		case entShootBot: spawn::robot("shooter bot", pos); break;
		case entGun: spawn::gun("gun", pos); break;
		}



	}
}

CEntity* CMap::getEntity(int idNo) {
	for (auto& entity : entities) {
		if (entity->id == idNo)
			return entity.get();
	}
	return nullptr;
}


