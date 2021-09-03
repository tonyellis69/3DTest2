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
	void onNotify(COnCursorNewHex& msg);
	bool isBlocked(CHex& pos, CHex& dest);
	void addSprite(std::shared_ptr<CEntity> sprite);
	void destroySprite(CEntity& sprite);
	void deleteEntity(CEntity& entity);
	void update(float dT);
	void togglePause();
	void onPlayerDeath();

	//CHex cursorPos;
	THexList cursorPath;
	CPlayerObject* player = nullptr;
	//std::shared_ptr< CPlayerObject> player;
	CMap* map;


	bool paused = false;

private:

};

extern CGameState game;