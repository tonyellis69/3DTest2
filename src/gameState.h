#pragma once

#include <string>
#include <memory>

#include "gameEvents.h"
#include "playerHexObj.h"
#include "gameHexArray.h"



/** A class encapsulating useful global state stuff everyone 
	needs direct access to. */
class CGameState   {
public:

	void setMap(CMap* map);
	bool isBlocked(CHex& pos, CHex& dest);
	void addSprite(std::shared_ptr<CEntity> sprite);
	void destroySprite(CEntity& sprite);
	void deleteEntity(CEntity& entity);
	void killEntity(CEntity& entity);
	void update(float dT);
	void togglePause();
	void toggleUImode(bool onOff);


	CPlayerObject* player = nullptr;
	//std::shared_ptr< CPlayerObject> player;
	CMap* map;


	bool paused = true;
	bool slowed = false;
	bool speeded = false;
	bool uiMode = false;

private:

};

extern CGameState game;