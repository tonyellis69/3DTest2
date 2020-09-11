#pragma once

#include "gameEvents.h"

enum TTurnPhase { playerPhase, robotPhase, playerDeadPhase };


/** A class encapsulating useful global state stuff. */
class CGameState : public CGameEventObserver  {
public:

	void setTurnPhase(TTurnPhase phase);
	TTurnPhase getTurnPhase();
	void onNotify(COnCursorNewHex& msg);

	CHex cursorPos;
	THexList cursorPath;
	bool onscreenRobotAction; ///<True if happened this turn.
private:
	TTurnPhase turnPhase;
	
};

extern CGameState world;