#pragma once

#include "ui/guiEvent.h"
#include "../gameEvent.h"

class CHexWorld;
/** Base class for the different hexWorld plug-in modules. */

class CGameMode {
public:
	CGameMode(CHexWorld* engine) {
		pHexWorld = engine;
	}
	virtual void guiHandler(CGUIevent& e) {}
	virtual void gameEventHandler(CGameEvent& e) {}

	virtual void makeMap() {}
	virtual void startProcTest() {}
	virtual void startGame() {}


	virtual void update(float dt) {}



	CHexWorld* pHexWorld;
	float dT;
};