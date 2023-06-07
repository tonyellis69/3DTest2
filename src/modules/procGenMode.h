#pragma once

#include "baseModule.h"

#include "../level/levelGen.h"

/** Module for experimenting with procedurally generated levels. */

class CProcGenMode : public CBaseModule {
public:
	CProcGenMode(CHexWorld* engine) : CBaseModule(engine) {}
	void onSpawn();

	void start();
	void restart();


	void guiHandler(CGUIevent& e);
	void gameEventHandler(CGameEvent& e);

	void makeLevel();
	

	void update(float dt);

	void writeGridToLevel();

	CRandLevel levelGen;

	glm::vec3 playerPos;
};