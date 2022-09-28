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
	void createTrap();
	void onLeftClick(bool stillPressed, int key);
	void onRightClick();
	void onCtrlRClick();
	void onCtrlLClick();
	void onUndo();
	void onRedo();
	void save();
	void load();
	void altWheel(float delta);
	void shapeWheel(float delta);
	void addEntity(glm::vec3& mousePos);
	void selectEntity(float delta);
	void selectShape(float delta);
	void onEntityMode(bool isOn);
	void onShapeMode(bool isOn);
	void onRightDrag();
	void onDelKey();
	void onMouseMove(glm::vec3& mouseWPos);

	std::string currentEntStr = "none"; 
	std::string currentShapeStr = "none";
	std::string entIdStr = "";
	bool entityMode = false;
	bool shapeMode = false;

private:
	void updateMap();

	CMap* pMap;
	CHexArray workingArray; 

	std::shared_ptr<CMapPatch> currentPatch;
	std::shared_ptr<CMapPatch> lastPatch;
	std::shared_ptr<CMapPatch> redoPatch;
	std::shared_ptr< CDeleteRect> deleteRect;

	CHex cursorHex;

	unsigned int currentEntity = 0;
	unsigned int currentShape = 0;
};

#define GLFW_KEY_LEFT_ALT           342


const int editNone = 0;
const int editPlayer = 1;
const int editMeleeBot = 2;
const int editShooterBot = 3;
const int editGun = 4;


const int editShapeNone = 0;
const int editShapeHex = 1;
const int editShapePara = 2;
const int editShapeRect = 3;
const int editShapeTri = 4;
const int editShapeTrap = 5;