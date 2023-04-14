#pragma once

#include "baseModule.h"

#include "../level/levelGen.h"

/** Module for experimenting with procedurally generated levels. */

class CProcGenMode : public CBaseModule {
public:
	CProcGenMode(CHexWorld* engine) : CBaseModule(engine) {}
	void init();

	void start();
	void restart();


	void guiHandler(CGUIevent& e);
	void gameEventHandler(CGameEvent& e);

	void makeMap();
	

	void update(float dt);

	CRandLevel levelGen;
};