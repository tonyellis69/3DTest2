#include "gameMode.h"

#include "../hexEngine.h"
#include "utils/files.h"
#include "win/win.h"

#include "../hexEngineEvent.h"
#include "../entity/cameraC.h"


void CGameMode::initalise() {
	if (gameWorld.entities.empty()) {
		gameWorld.loadLevel("manyMapTest.map");
		gameWorld.player->getComponent<CameraC>()->setHeight(15);
	}
	CWin::showMouse(true);
	CWin::fullScreen();
}



void CGameMode::restart() {
	//hardcoding for now
	gameWorld.loadLevel("manyMapTest.map");
}

void CGameMode::guiHandler(CGUIevent& e) {
	if (e.key == GLFW_KEY_ENTER && e.type == eKeyDown) {
		restart();
	}


}

void CGameMode::gameEventHandler(CGameEvent& e) {

}

void CGameMode::onSwitchTo() {
	CEntity* mainCam = gameWorld.getEntity("mainCam");
	if (mainCam) {
		mainCam->live = false;
	}
	gameWorld.player->getComponent<CameraC>()->enabled = true;
}


void CGameMode::update(float dt) {
	if (gameWorld.paused)
		return;

	this->dT = dt;

	if (gameWorld.slowed)
		this->dT = dt * 0.1f;
	if (gameWorld.speeded)
		this->dT = dt * 4.0f;

	//for (int n = 0; n < gameWorld.entities.size(); n++) {
	//	auto& entity = gameWorld.entities[n];
	//	if (entity->live)
	//		gameWorld.entities[n]->update(dT);
	//}

	//Needed for continous fire. GLFW callbacks only trigger once.
	if (CWin::mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
		gameWorld.player->playerC->onFireKey(true);
	}

	//handle movement
	if (CWin::keyPressed('A')) {
		if (CWin::keyPressed('W'))
			gameWorld.player->playerC->moveCommand(moveNW);
		else if (CWin::keyPressed('S'))
			gameWorld.player->playerC->moveCommand(moveSW);
		else
			gameWorld.player->playerC->moveCommand(moveWest);
	}
	else if (CWin::keyPressed('D')) {
		if (CWin::keyPressed('W'))
			gameWorld.player->playerC->moveCommand(moveNE);
		else if (CWin::keyPressed('S'))
			gameWorld.player->playerC->moveCommand(moveSE);
		else
			gameWorld.player->playerC->moveCommand(moveEast);
	}
	else if (CWin::keyPressed('W')) {
		gameWorld.player->playerC->moveCommand(moveNorth);
	}
	else if (CWin::keyPressed('S')) {
		gameWorld.player->playerC->moveCommand(moveSouth);
	}
}

