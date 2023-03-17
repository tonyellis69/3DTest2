#include "gameMode.h"

#include "../hexWorld.h"
#include "utils/files.h"
#include "win/win.h"

#include "../spawner.h"

void CGameMode::start() {

	loadLevel("manyMapTest.map");

	pHexWorld->physics.clearEntities();
	pHexWorld->physics.setMap(game.level->getHexArray());

	pHexWorld->hexRender.loadMap(game.level->getHexArray());
	pHexWorld->prepMapEntities();

	pHexWorld->setViewMode(gameView);

	game.paused = false;

	pHexWorld->followCam(pHexWorld->playerObj);


	pHexWorld->pBotZero = nullptr;
	for (auto& entity : game.entities) {
		if (entity->isRobot && entity->id == 7) {
			if (pHexWorld->pBotZero == NULL)
				pHexWorld->pBotZero = entity.get();
			else
				;// ((CRobot*)entity.get())->setState(robotDoNothing);
		}
	}
	CWin::showMouse(false);




	CWin::fullScreen();
}

void CGameMode::guiHandler(CGUIevent& e) {


	//!!mousewheel stuff:
	if (e.type == eMouseWheel) {
		
		if (CWin::keyPressed(GLFW_KEY_LEFT_SHIFT)) {
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
			if (game.paused)
				return;

			pHexWorld->mapDragging = false;
			pHexWorld->lastMousePos = pHexWorld->mousePos;
			pHexWorld->mousePos = e.pos;

			//CHex lastMouseHex = pHexWorld->hexCursor->transform->hexPosition;

			pHexWorld->calcMouseWorldPos();

		}


	if (e.key == GLFW_KEY_ENTER && e.type == eKeyDown ) {
		
		startGame();
	}





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


	if (!pHexWorld->editMode)
		for (int n = 0; n < game.entities.size(); n++) {
			auto& entity = game.entities[n];
			if (entity->live)
				game.entities[n]->update(dT);
		}

	//Needed for continous fire. GLFW callbacks only trigger once.
	if (CWin::mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
		game.player->onFireKey(true);
	}
}

void CGameMode::loadLevel(const std::string& fileName) {

	std::string fullPath = file::getDataPath() + fileName;// "oneMapTest.map";
	std::ifstream loadFile(fullPath);
	assert(loadFile.is_open());


	int version;
	file::readObject(version, loadFile);

	CMapHeader header;
	file::readObject(header, loadFile);

	int numHexes = header.height * header.width;

	auto level = std::make_unique<CLevel>();
	level->init(header.width, header.height);

	//FIXME: ugh
	TFlatArray flatArray(numHexes);
	for (auto& hex : flatArray) {
		file::readObject(hex, loadFile);
	}
	level->hexArray.setArray(flatArray);

	game.setLevel(std::move(level)); 
	

	int numEnts;
	file::readObject(numEnts, loadFile);
	for (int n = 0; n < numEnts; n++) {
		TEntityType entType;
		file::readObject(entType, loadFile);

		glm::vec3 pos;
		file::readObject(pos, loadFile);

		switch (entType) {
		case entPlayer: spawn::player("player", pos); break;
		case entMeleeBot: spawn::robot("melee bot", pos); break;
		case entShootBot: spawn::robot("shooter bot", pos); break;
		case entGun: spawn::gun("gun", pos); break;
		}



	}




	loadFile.close();


}
