
#include "mapEdit.h"

#include "hex/hex.h"


void CMapEdit::setMap(CGameHexArray* map) {
	pMap = map;
	currentPatch = nullptr;
	lastPatch = nullptr;
	redoPatch = nullptr;
	workingArray = pMap->getArray();

}

bool CMapEdit::mouseWheel(float delta, int key) {
	if (currentPatch == nullptr)
		return false;

	currentPatch->mouseWheel(delta, key);
	updateMap();

	return true;
}

void CMapEdit::onNewMouseHex(CHex& mouseHex) {
	cursorHex = mouseHex;
	updateMap();

}


void CMapEdit::createRing() {
	currentPatch = std::make_shared<CRingPatch>();
	currentPatch->create();
	
	updateMap();
}

/** Permanently add the current patch to the map. */
void CMapEdit::addEdit() {
	if (currentPatch == nullptr)
		return;

	if (lastPatch != nullptr) {
		for (auto& hex : lastPatch->hexes) {
			workingArray.getHexCube(CHex(hex.first) + lastPatch->offset) = hex.second;
		}

	}

	lastPatch = currentPatch;
	lastPatch->offset = cursorHex;

	currentPatch = nullptr;
	redoPatch = nullptr;

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

void CMapEdit::updateMap() {

	CHexArray tempArray = workingArray;

	if (lastPatch != nullptr) { //update map with last patch
		for (auto& hex : lastPatch->hexes) {
			tempArray.getHexCube(CHex(hex.first) + lastPatch->offset) = hex.second;
		}
	}


	if (currentPatch != nullptr) {
		for (auto& hex : currentPatch->hexes) {
			tempArray.getHexCube(CHex(hex.first) + cursorHex) = hex.second;
		}
	}

	pMap->setArray(tempArray);
	pMap->mapUpdated = true;
}
