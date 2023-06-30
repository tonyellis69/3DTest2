#pragma once

#include "ui/guiEvent.h"
#include "../gameEvent.h"
#include "../entity/entity.h"

enum TViewMode { gameView, devView, keepView };

class CHexEngine;
/** Base class for the different hexWorld plug-in modules. */

class CBaseModule {
public:
	CBaseModule(CHexEngine* engine) {
		pHexWorld = engine;
	}
	virtual void initalise() {}
	virtual void restart() {}
	virtual void guiHandler(CGUIevent& e) {}
	virtual void gameEventHandler(CGameEvent& e) {}

	
	virtual void startProcTest() {}
	virtual void startGame() {}

	virtual void update(float dt) {}
	void saveEntityState();
	//void restoreEntityState();

	TViewMode viewMode;///<Whether to use followcam, screen controls, etc, with this module

	CHexEngine* pHexWorld;
	float dT;

	//std::vector<CEntity> entityState; ///<Latest preserved entity states for this module
	TEntities entityState;


};