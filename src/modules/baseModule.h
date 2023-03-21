#pragma once

#include "ui/guiEvent.h"
#include "../gameEvent.h"

enum TViewMode { gameView, devView, keepView };

class CHexWorld;
/** Base class for the different hexWorld plug-in modules. */

class CBaseModule {
public:
	CBaseModule(CHexWorld* engine) {
		pHexWorld = engine;
	}
	virtual void start() {}
	virtual void guiHandler(CGUIevent& e) {}
	virtual void gameEventHandler(CGameEvent& e) {}

	
	virtual void startProcTest() {}
	virtual void startGame() {}

	virtual void update(float dt) {}

	TViewMode viewMode;///<Whether to use followcam, screen controls, etc, with this module

	CHexWorld* pHexWorld;
	float dT;


};