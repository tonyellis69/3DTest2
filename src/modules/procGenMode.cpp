#include "procGenMode.h"

//#include <glew.h> 
//#include "GLFW/glfw3.h"

#include "../hexWorld.h"

#include "win/win.h"

void CProcGenMode::init() {
	viewMode = keepView;
	makeLevel();

	CWin::showMouse(false);
	CWin::fullScreen();
}

/** Called every time we start a new session. */
void CProcGenMode::start() {

}

void CProcGenMode::restart(){
	//game.clearEntities();
	levelGen.reset();
	makeLevel();
}

void CProcGenMode::guiHandler(CGUIevent& e) {

		if (e.type == eMouseWheel ) {
			levelGen.resize(int(e.f2));
			makeLevel();
		}
		if (e.type == eKeyDown) {
			if (e.key == 'L') {
				levelGen.subdivide();
				//update hex grid of existing level
				levelGen.updateHexGrid();
				writeGridToLevel();
			}
			if (e.key == 'R') {
				levelGen.quadRemovals();
				makeLevel();
			}
			if (e.key == 'D') {
				levelGen.makeDoors();
				makeLevel();
			}
		}	

}

void CProcGenMode::gameEventHandler(CGameEvent& e) {

}





void CProcGenMode::makeLevel() {
	game.clearEntities();

	auto level = levelGen.makeLevel();

	//find player pos
	playerPos = levelGen.findPlayerPos();

	//ensure player spawns there
	level->entityRecs.push_back({ entPlayer,playerPos });

	game.setLevel(std::move(level));
	game.restoreEntities();

	writeGridToLevel();
}


void CProcGenMode::update(float dt) {
	if (game.paused)
		return;

	this->dT = dt;
}

void CProcGenMode::writeGridToLevel() {
	auto pArray = game.level->getHexArray();

	for (auto& hexLine : levelGen.hexLines) {
		for (auto& hex : hexLine) {
			pArray->getHexOffset(hex.x, hex.y).content = solidHex;
		}
	} 
}
