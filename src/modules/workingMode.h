#pragma once

#include "gameMode.h"

#include "../mapEdit/mapEdit.h"
#include "../level/levelGen.h"

/** Tempory module to replicate current hexWorld hodgepodge.
	To be split and deleted. */

class CWorkingMode : public CGameMode {
public:
	CWorkingMode(CHexWorld* engine) : CGameMode(engine) {}
	void guiHandler(CGUIevent& e);
	void gameEventHandler(CGameEvent& e);

	void makeMap();
	void startProcTest();
	void startGame();

	void update(float dt);

	void onMapDrag();
	void realtimeKeyChecks();
	void realtimeMouseButtons();
	


	CMapEdit mapEdit;
	bool procTestMode = false;
	CRandLevel levelGen;
	//bool editMode = false;
};