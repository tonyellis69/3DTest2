#include "gameMode.h"

#include "../hexWorld.h"
#include "utils/files.h"
#include "win/win.h"

#include "../spawner.h"

void CGameMode::start() {
	viewMode = gameView;

	if (entityState.empty()) {
		saveEntityState();
	}
	else {
		restoreEntityState();
	}

	CWin::showMouse(false);

	CWin::fullScreen();
}

void CGameMode::guiHandler(CGUIevent& e) {



}

void CGameMode::gameEventHandler(CGameEvent& e) {


}


void CGameMode::update(float dt)
{
	if (game.paused)
		return;

	this->dT = dt;

	if (game.slowed)
		this->dT = dt * 0.1f;
	if (game.speeded)
		this->dT = dt * 4.0f;


	
	for (int n = 0; n < game.entities.size(); n++) {
		auto& entity = game.entities[n];
		if (entity->live)
			game.entities[n]->update(dT);
	}

	//Needed for continous fire. GLFW callbacks only trigger once.
	if (CWin::mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
		game.player->playerC->onFireKey(true);
	}


	//handle movement
	if (CWin::keyPressed('A')) {
		if (CWin::keyPressed('W'))
			game.player->playerC->moveCommand(moveNW);
		else if (CWin::keyPressed('S'))
			game.player->playerC->moveCommand(moveSW);
		else
			game.player->playerC->moveCommand(moveWest);
	}
	else if (CWin::keyPressed('D')) {
		if (CWin::keyPressed('W'))
			game.player->playerC->moveCommand(moveNE);
		else if (CWin::keyPressed('S'))
			game.player->playerC->moveCommand(moveSE);
		else
			game.player->playerC->moveCommand(moveEast);
	}
	else if (CWin::keyPressed('W')) {
		game.player->playerC->moveCommand(moveNorth);
	}
	else if (CWin::keyPressed('S')) {
		game.player->playerC->moveCommand(moveSouth);
	}
}
