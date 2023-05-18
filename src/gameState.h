#pragma once

#include <string>
#include <memory>

//#include "gameEvents.h"
//#include "playerHexObj.h"
#include "level\level.h"



/** A class encapsulating useful global state stuff everyone 
	needs direct access to. */
class CGameState   {
public:

	void setLevel(CLevel* level);
	void setLevel(std::unique_ptr<CLevel> level);
	CLevel* getLevel();

	CEntities getEntitiesAt(const CHex& hex);
	CEntity* getEntity(int idNo);
	void addEntity(TEntity entity);
	void deleteEntity(CEntity& entity);
	void killEntity(CEntity& entity);
	void tidyEntityLists();
	void update(float dT);
	void save(std::ostream& out);
	void togglePause();
	void loadLevel(const std::string& fileName);
	void toggleUImode(bool onOff);

	void updatePlayerPtr();

	void restoreEntities();
	void clearEntities();

	CEntity* spawn(const std::string& name, glm::vec3& pos, float angle=0);


	//CPlayerObject* player = nullptr;
	CEntity* player = nullptr;

	std::unique_ptr<CLevel> level;


	bool paused = true;
	bool slowed = false;
	bool speeded = false;
	bool uiMode = false;

	std::unordered_map<std::string, CModel> models;
	std::unordered_map<std::string, std::vector<glm::vec4> >* pPalettes;

//private:

	TEntities entities; ///<The grand list of entities in the map.
	bool entitiesToDelete = false;
	bool entitiesToKill = false;

};

extern CGameState game;