#include "mapEdit.h"


#include <cassert>

#include "hex/hex.h"

#include "utils/files.h"


#include "../gameState.h" //for player obj, not ideal
#include "utils/log.h"



void CMapEdit::setMap(CLevel* map) {
	pMap = map;
	currentPatch = nullptr;
	lastPatch = nullptr;
	redoPatch = nullptr;
	deleteRect = nullptr;
	workingArray = *pMap->getHexArray();
	//spawn::pMap = map;
}

/** Currently called if the user mousewheels. */
bool CMapEdit::resize(float delta, int key) {
	if (currentPatch == nullptr)
		return false;
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

void CMapEdit::createTrap()
{
	currentPatch = std::make_shared<CTrapPatch>();
	currentPatch->create();
	currentPatch->setOffset(cursorHex);

	updateMap();
}


void CMapEdit::onLeftClick(bool stillPressed, int key) {
	if (deleteRect) {
		if (!stillPressed) {
			deleteRect->released = true;
			updateMap();
			return;
		}
	}



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
	if (deleteRect) {
		if (deleteRect->released)
			deleteRect.reset();
		else
			deleteRect->released = true;
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
	pMap->setHexArray(&workingArray);
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
	std::string filename = file::getDataPath() + "manyMapTest.map";
	std::ofstream saveFile(filename);
	updateMap();
	gameWorld.save(saveFile);
	saveFile.close();
}

void CMapEdit::load() {
	//std::string filename = file::getDataPath() + "manyMapTest.map";// "oneMapTest.map";
	//std::ifstream loadFile(filename);
	//assert(loadFile.is_open());
	//game.load(loadFile);
	//loadFile.close();
	gameWorld.loadLevel("manyMapTest.map");
	setMap(pMap);
	pMap->mapUpdated = true;
}

void CMapEdit::altWheel(float delta) {
	if (entityMode)
		selectEntity(delta);
	else {
		if (currentPatch != nullptr) {
			currentPatch->rotate(delta);
			updateMap();
		}
	}
}

/** If user holds shape key and mousewheels. */
void CMapEdit::shapeWheel(float delta) {
	if (shapeMode)
		selectShape(delta);
}

void CMapEdit::addEntity(glm::vec3& mousePos) {
	for (auto it = gameWorld.entities.begin(); it != gameWorld.entities.end();) {
		TEntity entity = *it;
		if (glm::distance(entity->getPos(), mousePos) < 0.5f) {
			gameWorld.deleteEntity(*it->get());
			if (gameWorld.player == (*it).get())
				gameWorld.player = nullptr;
			return;
		}
		it++;
	}

	switch (currentEntity) {
	case editPlayer: {
		if (gameWorld.player == nullptr)
			gameWorld.spawn("player", mousePos); 
		else {
			gameWorld.player->setPosition(mousePos);
		}
		break;
	}
	case editMeleeBot: gameWorld.spawn("melee bot", mousePos); break;
	case editShooterBot: gameWorld.spawn("shooter bot", mousePos); break;
	case editGun: gameWorld.spawn("gun", mousePos); break;

	}

}

/** Cycle through the entities to place. */
void CMapEdit::selectEntity(float delta) {
	currentEntity += (unsigned int)delta;
	currentEntity = (currentEntity) % 5;
	switch (currentEntity) {
	case editNone: currentEntStr = "none"; break;
	case editPlayer: currentEntStr = "player"; break;
	case editMeleeBot: currentEntStr = "melee bot"; break;
	case editShooterBot: currentEntStr = "shooter bot"; break;
	case editGun: currentEntStr = "gun"; break;
	}
}

void CMapEdit::selectShape(float delta) {
	currentShape += (unsigned int)delta;
	unsigned int choice = currentShape % 6;
	switch (choice) {
	case editShapeNone: currentShapeStr = "none"; currentPatch.reset(); updateMap(); break;
	case editShapeHex: currentShapeStr = "hex"; createRing();  break;
	case editShapePara: currentShapeStr = "parallelogram"; createParagram();  break;
	case editShapeRect: currentShapeStr = "rectangle"; createRect();  break;
	case editShapeTri: currentShapeStr = "triangle"; createTri(); break;
	case editShapeTrap: currentShapeStr = "trapezium"; createTrap();  break;

	}
}

void CMapEdit::onEntityMode(bool isOn) {
	if (currentPatch == nullptr) {
		entityMode = isOn;
		shapeMode = false;
	}
}

void CMapEdit::onShapeMode(bool isOn) {
	entityMode = false;
	shapeMode = isOn;
	if (isOn)
		selectShape(0);
}

/** Currently used for box-delete. */
void CMapEdit::onRightDrag() {
	if (currentPatch != nullptr)
		return;

	if (deleteRect == nullptr || deleteRect->released) {
		deleteRect = std::make_shared<CDeleteRect>();
		deleteRect->setOffset(cursorHex);
		deleteRect->create();


		updateMap();
		return;
	}

	//otherwise resize existing rect
	deleteRect->drag(cursorHex);
	updateMap();


}


void CMapEdit::onDelKey() {
	if (deleteRect) {
		deleteRect->findDelHexes();
		for (auto& hex : deleteRect->hexes) {
			workingArray.getHexCube(CHex(hex.first) + deleteRect->offset).content = emptyHex;
		}
		deleteRect.reset();
		updateMap();
	}

}

void CMapEdit::onMouseMove(glm::vec3& mouseWPos) {
	
	//on an entity?
	entIdStr = "";
	for (auto& it = gameWorld.entities.begin(); it != gameWorld.entities.end(); it++) {
		TEntity entity = *it;
		if (glm::distance(entity->getPos(), mouseWPos) < 0.5f) {
			entIdStr = std::to_string(entity->id);
			break;
		}
	}
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

	if (deleteRect != nullptr) {
		for (auto& hex : deleteRect->hexes) {
			tempArray.getHexCube(CHex(hex.first)  + deleteRect->offset) = hex.second;
		}
	}

	pMap->setHexArray(&tempArray);
	pMap->mapUpdated = true;
}
