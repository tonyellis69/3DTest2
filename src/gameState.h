#pragma once

#include "gameEvents.h"
#include "playerHexObj.h"
#include "gameHexArray.h"


enum TTurnPhase { playerPhase, robotPhase, playerDeadPhase };


/** A class encapsulating useful global state stuff. */
class CGameState : public CGameEventObserver  {
public:

	void setTurnPhase(TTurnPhase phase);
	void setMap(CGameHexArray* map);
	TTurnPhase getTurnPhase();
	void onNotify(COnCursorNewHex& msg);
	bool isBlocked(CHex& pos, CHex& dest);

	CHex cursorPos;
	THexList cursorPath;
	bool onscreenRobotAction; ///<True if happened this turn.
	CPlayerObject* player;
	CGameHexArray* map;

private:
	TTurnPhase turnPhase;

	
};

extern CGameState world;