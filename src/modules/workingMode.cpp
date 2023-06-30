#include "workingMode.h"

//#include <glew.h> 
//#include "GLFW/glfw3.h"

#include "../hexEngine.h"

#include "win/win.h"

void CWorkingMode::guiHandler(CGUIevent& e) {

	if (procTestMode) {
		if (e.type == eMouseWheel) {
			levelGen.resize(int(e.f2));
			makeMap();
			startProcTest();
		}
		if (e.type == eKeyDown) {
			if (e.key == 'L') {
				levelGen.subdivide();
				makeMap();
				startProcTest();
			}
			if (e.key == 'R') {
				levelGen.quadRemovals();
				makeMap();
				startProcTest();

			}
		}

	}


	//!!moved here:
	if (pHexWorld->editMode && e.type == eKeyDown) { //editmode key operations
		if (e.key == 'R') {
			mapEdit.createRing();
		}

		if (e.key == 'P') {
			mapEdit.createParagram();
		}

		if (e.key == 'C') {
			mapEdit.createRect();
		}

		if (e.key == 'T') {
			//mapEdit.createTri();
			mapEdit.createTrap();
		}

		if (e.key == 'S' && e.keyMod == eModCtrl)
			mapEdit.save();

		if (e.key == 'L' && e.keyMod == eModCtrl) {
			pHexWorld->physics.clearEntities();
			mapEdit.load();
		}

		if (e.key == GLFW_KEY_LEFT_ALT)
			mapEdit.onEntityMode(true);

		if (e.key == 'Z')
			mapEdit.onShapeMode(true);

		if (e.key == GLFW_KEY_DELETE)
			mapEdit.onDelKey();


		if (e.key == GLFW_KEY_LEFT_CONTROL) {
			gameWorld.toggleUImode(true);
		}
	}

	if (e.type == eKeyUp) {
		if (pHexWorld->editMode) {
			mapEdit.onEntityMode(false);
			mapEdit.onShapeMode(false);
		}
		else {
			if (e.key == GLFW_KEY_LEFT_CONTROL) {
				gameWorld.toggleUImode(false);
			}
		}

	}


	
	//!!mousewheel stuff:
	if (e.type == eMouseWheel) {
		if (pHexWorld->editMode) {
			if (CWin::keyPressed( GLFW_KEY_LEFT_ALT) ) {
				mapEdit.altWheel(e.dy);
				return;
			}
			if (mapEdit.shapeMode) {
				mapEdit.shapeWheel(e.dy);
				return;
			}
			if (!mapEdit.resize(e.dy, e.key)) {
				if (pHexWorld->hexRender.dollyCamera(e.dy * pHexWorld->zoomScale))
					pHexWorld->adjustZoomScale(e.dy);
			}
			return;
		}



		if (CWin::keyPressed(GLFW_KEY_LEFT_SHIFT) ) {
			pHexWorld->hexRender.pitchCamera(e.dy);
		}
		else {
			if (CWin::keyPressed(GLFW_KEY_LEFT_CONTROL)) {
				if (pHexWorld->hexRender.dollyCamera(e.dy * pHexWorld->zoomScale))
					pHexWorld->adjustZoomScale(e.dy);
			}
			else {
				;
			}
		}
	}

	//!!!mousemove stuff: 
	if (e.type == eMouseMove) {
		if (gameWorld.paused)
			return;

		pHexWorld->mapDragging = false;
		pHexWorld->lastMousePos = pHexWorld->mousePos;
		pHexWorld->mousePos = e.pos;

		//CHex lastMouseHex = pHexWorld->hexCursor->transform->hexPosition;

		pHexWorld->calcMouseWorldPos();

		if (CWin::mouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT) &&
			pHexWorld->mouseHex != pHexWorld->lastMouseHex && pHexWorld->editMode)
			mapEdit.onRightDrag();

		if (CWin::mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)  && pHexWorld->editMode ) {
			onMapDrag();
		}

		if (pHexWorld->editMode)
			mapEdit.onMouseMove(pHexWorld->mouseWorldPos);
	}

	if (e.key == GLFW_KEY_ENTER && e.type == eKeyDown && !pHexWorld->editMode) {
		makeMap();
		startGame();
	}




	if (pHexWorld->editMode) {
		if (e.type == eLeftUp) {
			if (e.keyMod == eModCtrl)
				mapEdit.onCtrlLClick();
			else if (e.keyMod == eModAlt)
				mapEdit.addEntity(pHexWorld->mouseWorldPos);
			else if (!pHexWorld->mapDragging)
				; // mapEdit.onLeftClick(stillPressed, mods); //FIXME needs rewriting
			}
			if (pHexWorld->mapDragging ) {
				pHexWorld->mapDragging = false;
				pHexWorld->cumulativeMapDrag = 0;
			}

			if (e.type == eRightDown) {
				if (e.keyMod == eModCtrl)
					mapEdit.onCtrlRClick();
				else
					mapEdit.onRightClick();
			}
			
	}
	

}

void CWorkingMode::gameEventHandler(CGameEvent& e) {
	//if (e.type == eGameEvent) {
		//catch player death here
	pHexWorld->onPlayerDeath();
	//	}

}



void CWorkingMode::makeMap() {
	//makemap old
	//level = new CLevel();
	//level->init(100, 40);
	auto level = std::make_unique<CLevel>();
	level->onSpawn(100, 40);


	for (int y = 0; y < 40; y++) {
		for (int x = 0; x < 100; x++) {
			level->getHexOffset(x, y).content = emptyHex;
		}
	}

	mapEdit.setMap(level.get());
	gameWorld.setLevel(std::move(level)); //added!

	//makemap new
	//auto level = levelGen.makeLevel();
	//game.setLevel(std::move(level));
}

void CWorkingMode::startProcTest() {
	procTestMode = true;
	pHexWorld->hexRender.loadMap(gameWorld.level.getHexArray());
	/*if (pHexWorld->hexCursor == NULL)
		pHexWorld->createCursorObject();*/
	if (pHexWorld->hexRender.camera.getPos() == glm::vec3(0))
		pHexWorld->setViewMode(gameView);
	else
		pHexWorld->setViewMode(keepView);
	gameWorld.paused = false;
	pHexWorld->freeCam();
	CWin::fullScreen();
//	pHexWorld->zoom2fit = true;
}

void CWorkingMode::startGame() {

	//game.setLevel(level); //old

	pHexWorld->physics.clearEntities();
	pHexWorld->physics.setMap(gameWorld.level.getHexArray());

	mapEdit.load();
	//!!!!!!!!!Previous point where map file was loaded

	pHexWorld->hexRender.loadMap(gameWorld.level.getHexArray());
	pHexWorld->prepMapEntities();

	//if (pHexWorld->hexCursor == NULL)
	//	pHexWorld->createCursorObject();

	pHexWorld->setViewMode(gameView);

	//gWin::pInv->refresh(); //can prob scrap, needs new system

	gameWorld.paused = false;

	pHexWorld->followCam(gameWorld.player);
	//freeCam(-76, 15);
//	toggleDirectionGraphics();
	//game.slowed = true;

	pHexWorld->pBotZero = nullptr;
	for (auto& entity : gameWorld.entities) {
		if (entity->isRobot && entity->id == 7) {
			if (pHexWorld->pBotZero == NULL)
				pHexWorld->pBotZero = entity.get();
			else
				;// ((CRobot*)entity.get())->setState(robotDoNothing);
		}
	}
	CWin::showMouse(false);

	//FIXME: this (and prob other stuff above) should be run once only in a hexWorld.init(). 
	/*pHexWorld->reticule = spawn::models["reticule"];
	pHexWorld->reticule.palette[0] = { 1,1,1,1 };*/


	CWin::fullScreen();

}







void CWorkingMode::update(float dt) {


	if (gameWorld.paused)
		return;

	this->dT = dt;

	if (gameWorld.slowed)
		this->dT = dt * 0.1f;
	if (gameWorld.speeded)
		this->dT = dt * 4.0f;


	if (!pHexWorld->editMode)
		for (int n = 0; n < gameWorld.entities.size(); n++) {
			auto& entity = gameWorld.entities[n];
			if (entity->live)
				gameWorld.entities[n]->update(dT);
		}


	realtimeKeyChecks();
	//realtimeMouseButtons();
}



void CWorkingMode::onMapDrag() {
	glm::vec3 mouseVec = pHexWorld->mouseWorldPos - pHexWorld->lastMouseWorldPos;
	//liveLog << "\n" << glm::length(mouseVec);
	pHexWorld->cumulativeMapDrag += glm::length(mouseVec);
	if (pHexWorld->cumulativeMapDrag < 0.5f)
		return;
	pHexWorld->hexRender.moveCamera(-mouseVec);
	//cumulativeMapDrag = 0;
	pHexWorld->mapDragging = true;
}

void CWorkingMode::realtimeKeyChecks() {
	if (CWin::keyPressed('T')) {
		startGame();
	}

	if (pHexWorld->cameraMode == camFree) {
		if (CWin::keyPressed('W')) pHexWorld->moveCamera(glm::vec3{ 0, 1, 0 });
		if (CWin::keyPressed('S')) pHexWorld->moveCamera(glm::vec3{ 0, -1, 0 });
		if (CWin::keyPressed('A')) pHexWorld->moveCamera(glm::vec3{ -1,0,0 });
		if (CWin::keyPressed('D')) pHexWorld->moveCamera(glm::vec3{ 1,0,0 });
	}
}

