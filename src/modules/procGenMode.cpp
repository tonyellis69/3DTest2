#include "procGenMode.h"

//#include <glew.h> 
//#include "GLFW/glfw3.h"

#include "../hexWorld.h"
#include "../spawner.h"

#include "win/win.h"

void CProcGenMode::init() {
	viewMode = keepView;
	CWin::showMouse(false);
	CWin::fullScreen();
}

/** Called every time we start a new session. */
void CProcGenMode::start() {
		makeMap();
}

void CProcGenMode::restart(){
	//game.clearEntities();
	levelGen.reset();
	start();
}

void CProcGenMode::guiHandler(CGUIevent& e) {

		if (e.type == eMouseWheel ) {
			levelGen.resize(int(e.f2));
			makeMap();
		}
		if (e.type == eKeyDown) {
			if (e.key == 'L') {
				levelGen.subdivide();
				makeMap();
			}
			if (e.key == 'R') {
				levelGen.quadRemovals();
				makeMap();
			}
		}	

}

void CProcGenMode::gameEventHandler(CGameEvent& e) {

}





void CProcGenMode::makeMap() {
	auto level = levelGen.makeLevel();
	game.setLevel(std::move(level));
}


void CProcGenMode::update(float dt) {
	if (game.paused)
		return;

	this->dT = dt;
}
