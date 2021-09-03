#pragma once

#include <memory>

#include "../gameHexArray.h"
#include "mapPatch.h"

/* For editing game maps. */
class CMapEdit {
public:
	void setMap(CMap* map);
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
	void altWheel(float delta);
	void addEntity(glm::vec3& mousePos);
	void selectEntity(float delta);
	void onEntityMode(bool isOn);
	void onLeftDrag();

	std::string currentEntStr = "none";
	bool entityMode = false;

private:
	void updateMap();

	CMap* pMap;
	CHexArray workingArray; 

	std::shared_ptr<CMapPatch> currentPatch;
	std::shared_ptr<CMapPatch> lastPatch;
	std::shared_ptr<CMapPatch> redoPatch;
	std::shared_ptr< CDeleteRect> deleteRect;

	CHex cursorHex;

	int currentEntity = 0;
};

#define GLFW_KEY_LEFT_ALT           342

const int editNone = 0;
const int editPlayer = 1;
const int editMeleeBot = 2;
const int editShooterBot = 3;