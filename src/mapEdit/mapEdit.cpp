#include "mapEdit.h"


#include <cassert>

#include "hex/hex.h"

#include "utils/files.h"

#include "../robot.h"

#include "../spawner.h"


void CMapEdit::setMap(CGameHexArray* map) {
	pMap = map;
	currentPatch = nullptr;
	lastPatch = nullptr;
	redoPatch = nullptr;
	workingArray = pMap->getArray();

}

bool CMapEdit::resize(float delta, int key) {
	if (currentPatch == nullptr)
		return false;
	if (key == GLFW_KEY_LEFT_ALT)
		currentPatch->rotate(delta);
	else
		currentPatch->resize(delta, key);
	updateMap();

	return true;
}

void CMapEdit::onNewMouseHex(CHex& mouseHex) {
	cursorHex = mouseHex;
	if (currentPatch)
		currentPatch->onNewMouseHex(mouseHex);
	updateMap();
}


void CMapEdit::createRing() {
	currentPatch = std::make_shared<CRingPatch>();
	currentPatch->create();
	currentPatch->setOffset(cursorHex);
	
	updateMap();
}

void CMapEdit::createParagram() {
	currentPatch = std::make_shared<CParagramPatch>();
	currentPatch->create();
	currentPatch->setOffset(cursorHex);

	updateMap();
}

void CMapEdit::createRect() {
	currentPatch = std::make_shared<CRectPatch>();
	currentPatch->create();
	currentPatch->setOffset(cursorHex);

	updateMap();
}

void CMapEdit::createTri() {
	currentPatch = std::make_shared<CTriPatch>();
	currentPatch->create();
	currentPatch->setOffset(cursorHex);

	updateMap();
}


void CMapEdit::onLeftClick() {
	if (lastPatch != nullptr) {
		for (auto& hex : lastPatch->hexes) {
			workingArray.getHexCube(CHex(hex.first) + lastPatch->offset) = hex.second;
		}
		lastPatch = nullptr;
	}

	if (currentPatch == nullptr) {
		int hexVal = workingArray.getHexCube(cursorHex).content;
		if (hexVal == emptyHex)
			workingArray.getHexCube(cursorHex).content = solidHex;
		else
			workingArray.getHexCube(cursorHex).content = emptyHex;
	}
	else {
		lastPatch = currentPatch;
		lastPatch->setOffset(cursorHex);
		currentPatch = nullptr;
		redoPatch = nullptr;
	}

	updateMap();
}

void CMapEdit::onRightClick() {
	if (currentPatch) {
		currentPatch = nullptr;
		updateMap();
	}
}

void CMapEdit::onCtrlRClick() {
	if (currentPatch != nullptr) {

		//first, add previous patch if any
		if (lastPatch != nullptr) {
			for (auto& hex : lastPatch->hexes) {
				workingArray.getHexCube(CHex(hex.first) + lastPatch->offset) = hex.second;
			}
		}

		//then use current patch as an eraser
		for (auto& hex : currentPatch->hexes) {
			workingArray.getHexCube(CHex(hex.first) + currentPatch->offset).content = emptyHex;
		}
	}

	currentPatch = nullptr;
	lastPatch = nullptr;
	pMap->setArray(workingArray);
	pMap->mapUpdated = true;
}

void CMapEdit::onCtrlLClick() {
	currentPatch = std::make_shared<CLinePatch>(cursorHex);
	currentPatch->create();

	updateMap();
}

void CMapEdit::onUndo() {
	redoPatch = lastPatch;
	lastPatch = nullptr;
	updateMap();
}

void CMapEdit::onRedo() {
	if (redoPatch == nullptr)
		return;
	lastPatch = redoPatch;
	redoPatch = nullptr;
	updateMap();
}

void CMapEdit::save() {
	std::string filename = file::getDataPath() + "mapTest.map";
	std::ofstream saveFile(filename);
	updateMap();
	pMap->save(saveFile);
	saveFile.close();
}

void CMapEdit::load() {
	std::string filename = file::getDataPath() + "mapTest.map";
	std::ifstream loadFile(filename);
	assert(loadFile.is_open());
	pMap->load(loadFile);
	loadFile.close();
	setMap(pMap);
	pMap->mapUpdated = true;
}

void CMapEdit::addRobot(glm::vec3& mousePos) {
	for (auto it = pMap->entities.begin(); it != pMap->entities.end();) {
		TEntity entity = *it;
		if (entity->isRobot &&
			glm::distance(entity->worldPos, mousePos) < 0.5f) {
			pMap->entities.erase(it);
			return;
		}
		it++;
	}

	spawn::robot("robot", mousePos);
}

void CMapEdit::updateMap() {

	CHexArray tempArray = workingArray;

	if (lastPatch != nullptr) { //update map with last patch
		for (auto& hex : lastPatch->hexes) {
			tempArray.getHexCube(CHex(hex.first) + lastPatch->offset) = hex.second;
		}
	}


	if (currentPatch != nullptr) {
		for (auto& hex : currentPatch->hexes) {
			tempArray.getHexCube(CHex(hex.first) + currentPatch->offset/* + cursorHex*/) = hex.second;
		}
	}

	pMap->setArray(tempArray);
	pMap->mapUpdated = true;
}
