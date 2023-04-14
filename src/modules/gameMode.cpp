#include "gameMode.h"

#include "../hexWorld.h"
#include "utils/files.h"
#include "win/win.h"

#include "../spawner.h"

void CGameMode::init() {
	//for now, hardcode reading file
	loadLevel("manyMapTest.map");
	viewMode = gameView;
	CWin::showMouse(false);

	CWin::fullScreen();
}

/** Called every time we start a new session. */
void CGameMode::start() {

	//spawn entity recs as entities in the game
	game.restoreEntities();
}

void CGameMode::restart() {
	game.clearEntities();
	start();
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


void CGameMode::loadLevel(const std::string& fileName) {
	std::string fullPath = file::getDataPath() + fileName;
	std::ifstream in(fullPath);
	assert(in.is_open());

	int version;
	file::readObject(version, in);

	CMapHeader header;
	file::readObject(header, in);

	int numHexes = header.height * header.width;

	auto newLevel = std::make_unique<CLevel>();
	newLevel->init(header.width, header.height);

	//FIXME: ugh
	TFlatArray flatArray(numHexes);
	for (auto& hex : flatArray) {
		file::readObject(hex, in);
	}
	newLevel->hexArray.setArray(flatArray);


	//this->level = std::move(newLevel);
	//entities.clear();



	int numEnts;
	file::readObject(numEnts, in);
	for (int n = 0; n < numEnts; n++) {
		TEntityType entType;
		file::readObject(entType, in);

		glm::vec3 pos;
		file::readObject(pos, in);

		newLevel->entityRecs.push_back({entType,pos});
	}

	in.close();

	game.setLevel(std::move(newLevel));

	CGameEvent e;
	e.type = gameLevelChange;
	lis::event(e);
}