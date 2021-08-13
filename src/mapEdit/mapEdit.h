#pragma once

#include <memory>

#include "../gameHexArray.h"
#include "mapPatch.h"

/* For editing game maps. */
class CMapEdit {
public:
	void setMap(CGameHexArray* map);
	bool mouseWheel(float delta, int key);
	void onNewMouseHex(CHex& mouseHex);
	void createRing();
	void addEdit();
	void onUndo();
	void onRedo();

private:
	void updateMap();

	CGameHexArray* pMap;
	CHexArray workingArray; 

	std::shared_ptr<CMapPatch> currentPatch;
	std::shared_ptr<CMapPatch> lastPatch;
	std::shared_ptr<CMapPatch> redoPatch;

	CHex cursorHex;

};