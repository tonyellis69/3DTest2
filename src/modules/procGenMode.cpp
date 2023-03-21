#include "procGenMode.h"

//#include <glew.h> 
//#include "GLFW/glfw3.h"

#include "../hexWorld.h"
#include "../spawner.h"

#include "win/win.h"

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



void CProcGenMode::start() {
	viewMode = keepView;

	if (!game.level)
		makeMap();

	game.paused = false;
	CWin::fullScreen();
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
