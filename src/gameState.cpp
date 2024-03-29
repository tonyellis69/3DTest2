#include "gameState.h"

//#include "robot.h"

#include "utils/files.h"
#include "utils/log.h"

#include "spawner.h"

#include "gameEvent.h"
#include "listen/listen.h"

CGameState gameWorld;


void CGameState::setLevel(CLevel* level) {
	//this->level =  std::unique_ptr<CLevel> (level);
	//TODO: scrap function!
}

void CGameState::setLevel(std::unique_ptr<CLevel> level) {
	//this->level = std::move(level);
	//CGameEvent e; 
	//e.type = gameLevelChange;
	//e.hexArray = level.hexArray;
	//lis::event(e);
	//TODO: scrap function!
}

/** Returns a short-term convenience pointer to the level. */
CLevel* CGameState::getLevel() {
	return &level;
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

CEntity* CGameState::getEntity(const std::string& name) {
	for (auto& entity : entities) {
		if (entity->name == name)
			return entity.get();
	}
	return nullptr;
}

std::shared_ptr<CEntity> CGameState::getEntitySmart(int idNo) {
	for (auto& entity : entities) {
		if (entity->id == idNo)
			return entity;
	}

	return std::shared_ptr<CEntity>();
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
//	liveLog << "\nEntity " << entity.id << " to kill";
	entity.live = false; 
	entity.deleteMe = true;
	gameWorld.entitiesToKill = true;
}

/** Remove any enities marked for deletion from the entity lists. */
void CGameState::tidyEntityLists() {
	if (entitiesToDelete) {
		/*sysLog << "\nNew destruction loop, entity count " << entities.size();
		for (auto& ent : entities)
			if (ent->id == 11)
				sysLog << "\nEnt 11 found!";*/

		for (auto& it = entities.begin(); it != entities.end(); ) {
			if (it->get()->deleteMe) {
				//sysLog << "\n...killing ent " << it->get()->id;
				it = entities.erase(it);
			}
			else
				++it;
		}

		entitiesToDelete = false;
		//sysLog << "\nLoop ends, entity count " << entities.size();
		//for (auto& ent : entities)
		//	if (ent->id == 11)
		//		sysLog << "\nEnt 11 still found!";

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
	CMapHeader header = { level.hexArray.width, level.hexArray.height };
	file::writeObject(header, out);
	for (auto& hex : level.hexArray.getFlatArray()) {
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


void CGameState::loadLevel(const std::string& fileName) {
	entities.clear();

	std::string fullPath = file::getDataPath() + fileName;
	std::ifstream in(fullPath);
	assert(in.is_open());

	int version;
	file::readObject(version, in);

	CMapHeader header;
	file::readObject(header, in);

	int numHexes = header.height * header.width;

	//auto newLevel = std::make_unique<CLevel>();
	level.onSpawn(header.width, header.height);

	//FIXME: ugh
	TFlatArray flatArray(numHexes);
	for (auto& hex : flatArray) {
		file::readObject(hex, in);
	}
	level.hexArray.setArray(flatArray);


//	this->level = std::move(newLevel);

	//TODO: create an entity to represent terrain collision


	int numEnts;
	file::readObject(numEnts, in);
	for (int n = 0; n < numEnts; n++) {
		TEntityType entType;
		file::readObject(entType, in);

		glm::vec3 pos;
		file::readObject(pos, in);
		switch (entType) {
		case entPlayer: player = gameWorld.spawn("player", pos); break;
		case entMeleeBot: gameWorld.spawn("melee bot", pos); break;
		case entShootBot: gameWorld.spawn("shooter bot", pos); break;
		case entGun: gameWorld.spawn("gun", pos); break;
		}
	}

	in.close();


	CGameEvent e(gameLevelChange);
	e.hexArray = &level.hexArray;
	lis::event(e);
}

void CGameState::toggleUImode(bool onOff) {
	uiMode = onOff;
}

void CGameState::updatePlayerPtr() {
	for (auto& ent : entities) {
		if (ent->entityType == entPlayer) {
			auto tmp = ent.get();
			player = tmp;
			return;
		}
	}
}

void CGameState::restoreEntities() {

	for (auto& entRec : level.entityRecs) {
		switch (entRec.entType) {
		case entPlayer: spawn("player", entRec.pos); break;
		case entMeleeBot: spawn("melee bot", entRec.pos); break;
		case entShootBot: spawn("shooter bot", entRec.pos); break;
		case entGun: spawn("gun", entRec.pos); break;
		}
	}

	for (auto& ent : entities) {
		ent->onSpawn();
	}

	updatePlayerPtr();
	//gameWorld.player->onSpawn(); //FIXME: fudge!!!!

	CGameEvent e(gameLevelChange);
	e.hexArray = &level.hexArray;
	lis::event(e);
	//FIXME:If we need this event at all, it should be gameEntitiesRestored or something
}

void CGameState::clearEntities() {
	entities.clear();
}

CEntity* CGameState::spawn(const std::string& name, glm::vec3& pos, float f1) {
	CEntity* ent = nullptr;

	if (name == "player")
		ent = spawn::player("player", pos); 
	else if (name == "melee bot")
		ent = spawn::robot("melee bot", pos); 
	else if (name == "shooter bot")
		ent = spawn::robot("shooter bot", pos);
	else if (name == "gun")
		ent = spawn::gun("gun", pos);
	else if (name == "missile")
		ent = spawn::missile("missile", pos, f1);
	else if (name == "explosion")
		ent = spawn::explosion("explosion", pos, f1);
	else if (name == "drop")
		ent = spawn::drop("drop", pos);
	else if (name == "mainCam")
		ent = spawn::mainCamera("mainCam", pos);

	if (ent) {
		CEntityEvent e = { entAdd,ent };
		lis::event(e);

	}

	return ent;
}

void CGameState::updateHexMap(CHexArray& newArray) {
	//entities.clear();
	level.onSpawn(newArray.width, newArray.height);
	level.hexArray.setArray(newArray);

	CGameEvent e(gameLevelChange);
	e.hexArray = &level.hexArray;
	lis::event(e);
}
