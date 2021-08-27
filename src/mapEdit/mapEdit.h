#pragma once

#include <memory>

#include "../gameHexArray.h"
#include "mapPatch.h"

/* For editing game maps. */
class CMapEdit {
public:
	void setMap(CGameHexArray* map);
	bool resize(float delta, int key);
	void onNewMouseHex(CHex& mouseHex);
	void createRing();
	void createParagram();
	void createRect();
	void createTri();
	void onLeftClick();
	void onRightClick();
	void onCtrlRClick();
	void onCtrlLClick();
	void onUndo();
	void onRedo();
	void save();
	void load();
	void addRobot(glm::vec3& mousePos);

private:
	void updateMap();

	CGameHexArray* pMap;
	CHexArray workingArray; 

	std::shared_ptr<CMapPatch> currentPatch;
	std::shared_ptr<CMapPatch> lastPatch;
	std::shared_ptr<CMapPatch> redoPatch;

	CHex cursorHex;

};

#define GLFW_KEY_LEFT_ALT           342