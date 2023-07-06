#include "procGenMode.h"

//#include <glew.h> 
//#include "GLFW/glfw3.h"

#include "../hexEngine.h"
#include "../entity/cameraC.h"

#include "win/win.h"

void CProcGenMode::initalise() {
	updateGameWorld();

	mainCam = gameWorld.spawn("mainCam");
	mainCam->getComponent<CameraC>()->setZoom2Fit(true);

	auto player = gameWorld.spawn("player");
	player->getComponent<CameraC>()->enabled = false;

	player->setPosition(levelGen.findPlayerPos());

	CWin::showMouse(true);
	CWin::fullScreen();
}


void CProcGenMode::restart(){
	//game.clearEntities();
	levelGen.reset();
	updateGameWorld();
}

void CProcGenMode::guiHandler(CGUIevent& e) {
		if (e.type == eMouseWheel ) {
			levelGen.resize(int(e.f2));
			updateGameWorld();
			mainCam->getComponent<CameraC>()->setZoom2Fit(true);
		}
		if (e.type == eKeyDown) {
			if (e.key == 'L') {
				levelGen.subdivide();
				updateGameWorld();
			}
			if (e.key == 'R') {
				levelGen.quadRemovals();
				updateGameWorld();
			}
			if (e.key == 'D') {
				levelGen.makeDoors();
				updateGameWorld();
			}
		}	

}

void CProcGenMode::gameEventHandler(CGameEvent& e) {

}





void CProcGenMode::updateGameWorld() {
	CHexArray tmpArray = levelGen.makeLevel();

	playerPos = levelGen.findPlayerPos();
	//ensure player spawns there
	//TODO

	gameWorld.updateHexMap(tmpArray);

}


void CProcGenMode::update(float dt) {


	this->dT = dt;


}


