#pragma once

#include "baseModule.h"

#include "ui/guiEvent.h"
#include "../gameEvent.h"



/** The hexWorld plugin that provides the actual game. */

class CGameMode : public CBaseModule {
public:
	CGameMode(CHexWorld* engine) : CBaseModule(engine) {}
	void init();
	void start();
	void restart();
	void guiHandler(CGUIevent& e);
	void gameEventHandler(CGameEvent& e);

	virtual void makeMap() {}
	virtual void startProcTest() {}
	virtual void startGame() {}


	void update(float dt);


	void loadLevel(const std::string& fileName);
	

};