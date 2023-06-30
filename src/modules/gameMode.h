#pragma once

#include "baseModule.h"

#include "ui/guiEvent.h"
#include "../gameEvent.h"



/** The hexWorld plugin that provides the actual game. */

class CGameMode : public CBaseModule {
public:
	CGameMode(CHexEngine* engine) : CBaseModule(engine) {}
	void initalise();
	void restart();
	void guiHandler(CGUIevent& e);
	void gameEventHandler(CGameEvent& e);

	virtual void makeMap() {}
	virtual void startProcTest() {}
	virtual void startGame() {}


	void update(float dt);


	

};