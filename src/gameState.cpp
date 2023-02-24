#include "gameState.h"

#include "robot.h"

#include "utils/files.h"
#include "spawner.h"


CGameState game;


void CGameState::setLevel(CLevel* level) {
	//std::unique_ptr<CLevel> ptr(level);
	//this->level = std::move(ptr);
	this->level =  std::unique_ptr<CLevel> (level);
}

void CGameState::setLevel(std::unique_ptr<CLevel> level)
{
	this->level = std::move(level);
}


//Temporary fix - consider map etc for speed, quadtree
CEntities CGameState::getEntitiesAt(const CHex& hex)
{
	CEntities hexEntities;
	for (auto& entity : entities) {
		if (entity->transform->hexPosition == hex)
			hexEntities.push_back(entity.get());
	}

	return hexEntities;
}

CEntity* CGameState::getEntity(int idNo) {
	for (auto& entity : entities) {
		if (entity->id == idNo)
			return entity.get();
	}
	return nullptr;
}

/** Basic add-to-world without specifying a location. For items in
	posession of another entity, not yet in play, etc.*/
void CGameState::addEntity(TEntity entity) {
	entities.push_back(entity);
}

/** Tag an entity for immediate deletion from the map. */
void CGameState::deleteEntity(CEntity& entity) {
	entity.deleteMe = true;
	entitiesToDelete = true;
}



void CGameState::killEntity(CEntity& entity) {
	entity.live = false; 
	entity.deleteMe = true;
	game.entitiesToKill = true;
}

/** Remove any enities marked for deletion from the entity lists. */
void CGameState::tidyEntityLists() {
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

void CGameState::update(float dT) {

	tidyEntityLists();

}

/** Save the map details to file. */
void CGameState::save(std::ostream& out) {
	int version = 1;
	file::writeObject(version, out);

	//TO DO: hand the level-saving off to CLevel.save(out) - hexArray should be invisible at this level
	CMapHeader header = { level->hexArray.width, level->hexArray.height };
	file::writeObject(header, out);
	for (auto& hex : level->hexArray.getFlatArray()) {
		file::writeObject(hex, out);
	}

	int numEnts = entities.size();
	file::writeObject(numEnts, out);
	for (auto& ent : entities) {
		file::writeObject(ent->entityType, out);
		file::writeObject(ent->getPos(), out);
	}
}

void CGameState::togglePause() {
	paused = !paused;
}


void CGameState::load(std::istream& in) {
	int version;
	file::readObject(version, in);

	CMapHeader header;
	file::readObject(header, in);

	int numHexes = header.height * header.width;
	TFlatArray flatArray(numHexes);
	for (auto& hex : flatArray) {
		file::readObject(hex, in);
	}
	level->hexArray.setArray(flatArray);

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

void CGameState::toggleUImode(bool onOff) {
	uiMode = onOff;
}
