#pragma once

#include <string>
#include <memory>

#include "gameEvents.h"
#include "playerHexObj.h"
#include "gameHexArray.h"


#include "sprite.h"


enum TTurnPhase { playerPhase, robotPhase, playerDeadPhase };


/** A class encapsulating useful global state stuff everyone 
	needs direct access to. */
class CGameState : public CGameEventObserver  {
public:

	void setTurnPhase(TTurnPhase phase);
	void setMap(CGameHexArray* map);
	TTurnPhase getTurnPhase();
	void onNotify(COnCursorNewHex& msg);
	bool isBlocked(CHex& pos, CHex& dest);
	void destroySprite(CSprite& sprite);
	void deleteEntity(CGameHexObj& entity);
	void update(float dT);


	CHex cursorPos;
	THexList cursorPath;
	bool onscreenRobotAction; ///<True if happened this turn.
	CPlayerObject* player;
	CGameHexArray* map;



	std::vector<std::shared_ptr<CSprite>> sprites;

private:
	TTurnPhase turnPhase;

	std::vector<CSprite*> spriteDeathlist;
};

extern CGameState world;