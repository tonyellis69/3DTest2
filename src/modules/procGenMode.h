#pragma once

#include "baseModule.h"

#include "../level/levelGen.h"

/** Module for experimenting with procedurally generated levels. */

class CProcGenMode : public CBaseModule {
public:
	CProcGenMode(CHexEngine* engine) : CBaseModule(engine) {}
	void initalise();

	void restart();


	void guiHandler(CGUIevent& e);
	void gameEventHandler(CGameEvent& e);

	void updateGameWorld();
	

	void update(float dt);

private:
	CRandLevel levelGen;

	glm::vec3 playerPos;
	CEntity* mainCam;
};