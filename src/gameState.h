#pragma once

#include <string>
#include <memory>

#include "gameEvents.h"
#include "playerHexObj.h"
#include "gameHexArray.h"



/** A class encapsulating useful global state stuff everyone 
	needs direct access to. */
class CGameState : public CGameEventObserver  {
public:

	void setMap(CGameHexArray* map);
	void onNotify(COnCursorNewHex& msg);
	bool isBlocked(CHex& pos, CHex& dest);
	void addSprite(std::shared_ptr<CEntity> sprite);
	void destroySprite(CEntity& sprite);
	void deleteEntity(CEntity& entity);
	void update(float dT);
	void togglePause();

	CHex cursorPos;
	THexList cursorPath;
	bool onscreenRobotAction; ///<True if happened this turn.
	CPlayerObject* player;
	CGameHexArray* map;


	bool paused = false;

private:

};

extern CGameState world;