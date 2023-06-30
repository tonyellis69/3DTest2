#pragma once

#include "baseModule.h"

#include "../level/levelGen.h"

/** Module for experimenting with procedurally generated levels. */

class CProcGenMode : public CBaseModule {
public:
	CProcGenMode(CHexEngine* engine) : CBaseModule(engine) {}
	void initalise();

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