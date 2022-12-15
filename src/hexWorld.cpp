#include "hexWorld.h"

#include <string>
#include <algorithm>

#include <glm/gtx/vector_angle.hpp> 

#include "UI\gui.h"

#include "utils/log.h"
#include "gameTextWin.h"
#include "gameState.h"
#include "hexRenderer.h"

#include "sound/sound.h"

#include "UI/uiRender.h" //temp debug

#include "messaging/msg2.h" //for test

#include "gameGui.h"
 
#include "UI/fonts.h" //temp test

#include "renderer/imRendr/imRendr.h"

#include "GLFW/glfw3.h"

#include "spawner.h"

#include "hexRender/drawFunc.h"

#include "hexRender/destGraphic.h"
#include "hexRender/avoidGraphic.h"

#include "UI/gui2.h"

#include "win/win.h"

#include "UI/guiEvent.h"
#include "gameEvent.h"

#include "items/shield.h"

CHexWorld::CHexWorld() {
	game.paused = true;

	hexRender.init();

	hexRendr2.init();
	hexRender.pCamera = &hexRendr2.camera;
	//hexRender.pLineShader = hexRendr2.lineShader;

	imRendr::setMatrix(&hexRendr2.camera.clipMatrix);
	
	lis::subscribe<CGUIevent>(this);
	lis::subscribe<CGameEvent>(this);

	hexPosLbl = gui::addLabel("xxx", 10, 10);
	hexPosLbl->setTextColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	snd::setVolume(1);

	gWin::initWindows();

	initPalettes();

	//lis::unsubscribe<CEvent>(this);
}

void CHexWorld::onEvent(CGUIevent& e) {
	if (e.type == eKeyDown) {
		if (e.i1 == GLFW_KEY_F2)
			toggleDirectionGraphics();
		else if (e.i1 == 'F')
			game.slowed = !game.slowed;
		else if (e.i1 == 'G')
			game.speeded = !game.speeded;
		else if (e.i1 == GLFW_KEY_F3)
			hexRender.tmpX = hexRender.tmpX == 0 ? 1 : 0;

		else if (e.i1 == '-')
			hexRender.tmpLineThickness -= 0.25f;
		else if (e.i1 == '=')
			hexRender.tmpLineThickness += 0.25f;
		else if (e.i1 == '[')
			hexRender.tmpLineSmooth -= 0.05f;
		else if (e.i1 == ']')
			hexRender.tmpLineSmooth += 0.05f;
		else if (e.i1 == 'O')
			hexRender.tmpLineSolid -= 0.05f;
		else if (e.i1 == 'P')
			hexRender.tmpLineSolid += 0.05f;

		else if (e.i1 == 'L')
			hexRender.tmpKernel -= 2;
		else if (e.i1 == ';')
			hexRender.tmpKernel += 2;
		else if (e.i1 == '\'')
			hexRender.tmpSigma -= 0.1f;
		else if (e.i1 == '\\')
			hexRender.tmpSigma += 0.1f;
		else if (e.i1 == ',')
			hexRender.tmpBlurs -= 1;
		else if (e.i1 == '.')
			hexRender.tmpBlurs += 1;
		else if (e.i1 == '/')
		{
			hexRender.tmpBlurTextDivisor++;
			if (hexRender.tmpBlurTextDivisor > 5)
				hexRender.tmpBlurTextDivisor = 1;
			hexRender.resizeBlurTextures();
		}
		else if (e.i1 == 'N')
			hexRender.tmpFade -= 0.05f;
		else if (e.i1 == 'M')
			hexRender.tmpFade += 0.05f;

		else if (e.i1 == 'R')
			hexRender.recompileShader();



	}



}

void CHexWorld::onEvent(CGameEvent& e) {
	//if (e.type == eGameEvent) {
		//catch player death here
		onPlayerDeath();
//	}

}



/**	Load a multipart mesh for storage under the given name. */
void CHexWorld::addMesh(const std::string& name, const std::string& fileName) {
	CImporter importer;
	importer.loadFile(fileName);	
	spawn::models[name] = importer.getModel();
}

/** Import a mesh to use in drawing the level. */
void CHexWorld::addHexTile(const std::string& name, const std::string& fileName, std::vector<glm::vec4>& colours) {
	CImporter importer;
	importer.loadFile(fileName);

	hexRender.addHexTile(name, importer.getVertData(),colours);

}

/** Create a map using the data in the given Tig file. */
void CHexWorld::makeMap() {

	//map = mapMaker.makeMap(tigMap);

	map = new CMap();
	map->init(100,40);

	for (int y = 0; y < 40; y++) {
		for (int x = 0; x < 100; x++) {
			map->getHexOffset(x, y).content = emptyHex;
		}
	}

	hexRendr2.setMap(map->getHexArray());
	mapEdit.setMap(map);
}

void CHexWorld::deleteMap() {
	delete map;
}


/** Required each time we restart. */
void CHexWorld::startGame() {

	hexRendr2.setMap(map->getHexArray());

	game.setMap(map);
	physics.clearEntities();

	physics.setMap(map->getHexArray());

	mapEdit.load();
	hexRender.loadMap(map->getHexArray());
	prepMapEntities();

	if (hexCursor == NULL)
		createCursorObject();

	map->getHexArray()->effectsNeedUpdate = true;

	setViewMode(gameView);

	map->getHexArray()->effectsNeedUpdate = true; //old code! Replace

	gWin::pInv->refresh();

	game.paused = false;

	followCam(playerObj);
	//freeCam(-76, 15);
//	toggleDirectionGraphics();
	//game.slowed = true;

	pBotZero = nullptr;
	for (auto& entity : map->entities) {
		if (entity->isRobot && entity->id == 7) {
			if (pBotZero == NULL)
				pBotZero = entity.get();
			else
				;// ((CRobot*)entity.get())->setState(robotDoNothing);
		}
	}
	CWin::showMouse(false);

	//FIXME: this (and prob other stuff above) should be run once only in a hexWorld.init(). 
	reticule = spawn::models["reticule"]; 
	reticule.palette[0] = { 1,1,1,1 };


	CWin::fullScreen();

}


void CHexWorld::moveCamera(glm::vec3& direction) {
	float camSpeed =  10 * dT;
	glm::vec3 vector = direction * camSpeed;
	//hexRendr2.moveCamera(vector); //FIX: phase out!
	freeCamPos.x += vector.x; freeCamPos.y += vector.y;
}

/** Called *when* a key is pressed. */
void CHexWorld::onKeyDown(int key, long mod) {

	//temp user interface stuff!
	msg::emit(msg::tmpMsg, key); //temp!

	if (editMode) { //editmode key operations
		if (key == 'R') {
			mapEdit.createRing();
		}

		if (key == 'P') {
			mapEdit.createParagram();
		}

		if (key == 'C') {
			mapEdit.createRect();
		}

		if (key == 'T') {
			//mapEdit.createTri();
			mapEdit.createTrap();
		}

		if (key == 'S' && mod == GLFW_MOD_CONTROL)
			mapEdit.save();

		if (key == 'L' && mod == GLFW_MOD_CONTROL) {
			physics.clearEntities();
			mapEdit.load();
		}

		if (key == GLFW_KEY_LEFT_ALT)
			mapEdit.onEntityMode(true);

		if (key == 'Z')
			mapEdit.onShapeMode(true);

		if (key == GLFW_KEY_DELETE)
			mapEdit.onDelKey();
		return;
	}



//	if (key == 'I')
	//	playerObj->showInventory();



	if (key == 'F') {
		hexRendr2.toggleFollowCam();
		hexRendr2.followTarget(playerObj->worldPos);
	}

	if (key == 'R') {
		hexRendr2.hexLineShader->recompile();
		hexRendr2.lineShader->recompile();
		hexRendr2.hexSolidShader->recompile();
		hexRendr2.visibilityShader->recompile();
	}

	if (key == 'L') {
		lineOfSight = !lineOfSight;

	}

	if (key == GLFW_KEY_LEFT_CONTROL) {
		game.toggleUImode(true);
	}
 
}

void CHexWorld::onKeyUp(int key, long mod) {
	if (editMode) {
		mapEdit.onEntityMode(false);
		mapEdit.onShapeMode(false);
	}
	else {
		if (key == GLFW_KEY_LEFT_CONTROL) {
			game.toggleUImode(false);
		}
	}
}

void CHexWorld::onMouseWheel(float delta, int key) {
	if (editMode) {
		if (key == GLFW_KEY_LEFT_ALT) {
			mapEdit.altWheel(delta);
			return;
		}
		if (mapEdit.shapeMode) {
			mapEdit.shapeWheel(delta);
			return;
		}
		if (!mapEdit.resize(delta, key)) {
			if (hexRendr2.dollyCamera(delta * zoomScale))
				adjustZoomScale(delta);
		}
		return;
	}



	if (key == GLFW_KEY_LEFT_SHIFT) {
		hexRendr2.pitchCamera(delta);
	}
	else {
		if (key == GLFW_KEY_LEFT_CONTROL) {
			if (hexRendr2.dollyCamera(delta * zoomScale) )
				adjustZoomScale(delta);
		}
		else {
			;
		}
	}
}

void CHexWorld::onMouseMove(int x, int y, int key) {
	if (game.paused)
		return;

	mapDragging = false;
	lastMousePos = mousePos;
	mousePos = { x,y };

	CHex lastMouseHex = hexCursor->hexPosition;

	calcMouseWorldPos();
	
	if (key == GLFW_MOUSE_BUTTON_RIGHT && 
		hexCursor->hexPosition != lastMouseHex &&  editMode)
		mapEdit.onRightDrag();

	if (key == GLFW_MOUSE_BUTTON_LEFT && editMode) {
		onMapDrag();
	}

	if (editMode)
		mapEdit.onMouseMove(mouseWorldPos);
}

void CHexWorld::calcMouseWorldPos() {
	auto [mouseHex, mouseWS] = hexRendr2.pickHex(mousePos.x, mousePos.y);
	lastMouseWorldPos = mouseWorldPos;
	mouseWorldPos = mouseWS;
	glm::vec3 mouseVec = mouseWorldPos - playerObj->worldPos;
	playerObj->setMouseDir(glm::normalize(mouseVec));

	if (mouseHex != hexCursor->hexPosition) {
		CMouseExitHex msg;
		msg.leavingHex = hexCursor->hexPosition;
		onNewMouseHex(mouseHex);
	}
}


void CHexWorld::draw() {
	hexRender.drawMap();

	hexRender.resetDrawLists();

	for (auto& entity : map->entities) {
		if (entity->name == "basicShield")
			int b = 0;
		if (entity->live) 
			//entity->drawFn.get()->draw(hexRender);
			entity->modelCmp->draw(hexRender);
	}
	if (directionGraphicsOn) {
		for (auto& graphic : hexRender.graphics)
			graphic->draw(hexRender);
	}


	//hexRender.drawLineListDBG();

	hexRender.makeGlowShapes();

	hexRender.blur();

	hexRender.drawGlow();


	//draw masks first
	hexRender.drawMaskList();

	hexRender.drawLineList(); 

	hexRender.drawSolidList();
	hexRender.drawUpperLineList();

	hexRender.drawExplosionList();

	if (!game.player->dead) {
		//imRendr::setDrawColour({ 1.0f, 1.0f, 1.0f, 1.0f });
		//imRendr::drawLine(playerObj->worldPos, mouseWorldPos);
		//hexCursor->draw();
		drawReticule();
	}

	int sh = ((CShieldComponent*)game.player->shield->item.get())->hp;

	imRendr::drawText(600, 50, "HP: " + std::to_string(game.player->hp)  
		+ " Shield: " + std::to_string(sh)

	);

	//imRendr::drawText(300, 70, "thickness: " + std::to_string(hexRender.tmpLineThickness)
	//	+ " smoothing: " + std::to_string(hexRender.tmpLineSmooth));
	//imRendr::drawText(300, 90, "kernel: " + std::to_string(hexRender.tmpKernel)
	//	+ " sigma: " + std::to_string(hexRender.tmpSigma) + " blurs: "
	//	+ std::to_string(hexRender.tmpBlurs)
	//	+ " blur div: " + std::to_string(hexRender.tmpBlurTextDivisor)
	//	+ " solidity: " + std::to_string(hexRender.tmpLineSolid));

	if (editMode) {
		imRendr::drawText(600, 80, "Entity: " +  mapEdit.currentEntStr + " shape: " + mapEdit.currentShapeStr +
		mapEdit.entIdStr);

	}

	if (pBotZero && pBotZero->toRemove == false)
		imRendr::drawText(600, 80, pBotZero->diagnostic);

}

/** Adjust horizontal vs vertical detail of the view. Usually called when the screen size changes. */
void CHexWorld::setAspectRatio(glm::vec2& ratio) {
	float defFov = glm::radians(45.0f);
	float defaultScreenH = 800.0f;
	float newFov = asin(sin(defFov / 2.0f) * ratio.y / defaultScreenH) * 2.0f;
	hexRendr2.setCameraAspectRatio(ratio,newFov);
	hexRender.setScreenSize(ratio);
}


/** Called every frame to get the hex world up to date.*/
void CHexWorld::update(float dt) {

	if (game.paused)
		return;

	this->dT = dt;

	if (game.slowed)
		this->dT = dt * 0.1f;
	if (game.speeded)
		this->dT = dt * 4.0f;


	updateCameraPosition();

	calcMouseWorldPos();

	physics.update(dT);

	if (!editMode)
		for (int n = 0; n < map->entities.size(); n++) {
			auto& entity = map->entities[n];
			if (entity->live)
				map->entities[n]->update(dT);
		}




	//if (game.map->entitiesToDelete)
	//	physics.removeDeletedEntities();

	////TO DO: this should come to replace above
	//if (game.map->entitiesToKill) {
	//	physics.removeDeadEntities();
	//	game.map->entitiesToKill = false;
	//}


	game.update(dT);

	if (map->mapUpdated) {
		//hexRendr2.setMap(map->getHexArray()); //temp to refresh map
		hexRender.loadMap(map->getHexArray());
		map->mapUpdated = false;
	}

	gWin::update(dT);

	for (auto& graphic : hexRender.graphics) {
		graphic->update(dT);
	}


	realtimeKeyChecks();
	realtimeMouseButtons();

	removeEntities();

}



/** User has triggered the ctrl left mouse  event. */
void CHexWorld::onCtrlLMouse() {
	
}


/** Player has pressed the enter key. */
void CHexWorld::enterKeyDown() {
	deleteMap();
	//entitiesToDraw.clear();

	makeMap();
	startGame();
}

void CHexWorld::toggleView() {
	if (viewMode == gameView)
		setViewMode(devView);
	else
		setViewMode(gameView);
}

void CHexWorld::toggleEditMode() {
	editMode = !editMode;
	if (editMode) {
		freeCam();
		gWin::pNear->hideWin();
		CWin::showMouse(true);
	}
	else {
		followCam(playerObj);
		gWin::pNear->showWin();
		prepMapEntities();
		CWin::showMouse(false);
	}
	
}

void CHexWorld::onUndo() {
	if (editMode)
		mapEdit.onUndo();
}

void CHexWorld::onRedo() {
	if (editMode)
		mapEdit.onRedo();
}


/** Plug the map's entities into physics and whatever else they need
	to be connected to. */
void CHexWorld::prepMapEntities() {
	physics.clearEntities();

	for (auto& entity : map->entities) {
		if (entity->isRobot)
			physics.add(entity.get());

		if (entity->entityType == entPlayer) {
			game.player = (CPlayerObject*)entity.get();
			playerObj = (CPlayerObject*)entity.get();
			physics.add(entity.get());
		}


	}

}




/////////////public - private devide

void CHexWorld::createCursorObject() {
	hexCursor = new CEntity();

	//hexCursor->setModel(spawn::models["cursor"]);
	hexCursor->setPosition(CHex(0, 0, 0));
}


/** Respond to mouse cursor moving to a new hex. */
void CHexWorld::onNewMouseHex(CHex& mouseHex) {
	hexCursor->setPosition(mouseHex);


	/*if (!lineOfSight)
		cursorPath = map->aStarPath(playerObj->hexPosition, hexCursor->hexPosition, true);
	else
		cursorPath = *hexLine2(playerObj->hexPosition, hexCursor->hexPosition);*/


	std::stringstream coords; coords << "cube " << mouseHex.x << ", " << mouseHex.y << ", " << mouseHex.z;
	glm::i32vec2 offset = cubeToOffset(mouseHex);
	coords << "  offset " << offset.x << ", " << offset.y;
	glm::i32vec2 index = map->getHexArray()->cubeToIndex(mouseHex);
	coords << " index " << index.x << " " << index.y;
	glm::vec3 worldSpace = cubeToWorldSpace(mouseHex);
	coords << " worldPos " << worldSpace.x << " " << worldSpace.y << " " << worldSpace.z;
	coords << " " << map->getHexArray()->getHexCube(mouseHex).content;
	//glm::vec2 screenPos = hexRenderer.worldPosToScreen(worldSpace);
	coords << " wsMouse " << mouseWorldPos.x << " " << mouseWorldPos.y;
	coords << " scrnMouse " << mousePos.x << " " << mousePos.y;
//	CHex test = worldSpaceToHex3(glm::vec3(mouseWorldPos.x, mouseWorldPos.y,0));
//	coords << " rect " << test.x << " " << test.z;


	hexPosLbl->setText(coords.str());

	//lblText = coords.str();

	if (editMode)
		mapEdit.onNewMouseHex(mouseHex);

}




void CHexWorld::onFireKey(bool stillPressed, int mods) {
	if (editMode) {
		if (!stillPressed) {
			if (mods == GLFW_MOD_CONTROL)
				mapEdit.onCtrlLClick();
			else if (mods == GLFW_MOD_ALT)
				mapEdit.addEntity(mouseWorldPos);
			else if (!mapDragging)
				mapEdit.onLeftClick(stillPressed, mods);
		}
		if (mapDragging && !stillPressed) {
			mapDragging = false;
			cumulativeMapDrag = 0;
		}
	}
	//else if (!game.uiMode)
	//	playerObj->onFireKey(stillPressed);
}

void CHexWorld::onRightKey(bool released, int mods) {
	if (editMode) {
		if (mods == GLFW_MOD_CONTROL)
			mapEdit.onCtrlRClick();
		else
			mapEdit.onRightClick();
	}
}


/** Handle an 'external function call' from Tig. */
int CHexWorld::tigCall(int memberId) {
	//std::string strParam;
	//switch (memberId) {
	//case tig::msgFn : 
	//	strParam = vm->getParamStr(0);
	//	liveLog << strParam; break;
	//case tig::popupWin :
	//	strParam = vm->getParamStr(0);
	//	;// mothballing this, use messaging popupMsg(strParam); break;
	//}
	return 1;
}


void CHexWorld::updateCameraPosition() {
	//if (viewMode == gameView && !editMode && !game.player->dead )
		//hexRendr2.followTarget(playerObj->worldPos);
		//old version
	
	if (cameraMode == camFollow) {
		if (pFollowCamEnt) {
			 hexRender.setCameraPos(pFollowCamEnt->worldPos.x, pFollowCamEnt->worldPos.y);
		}


	}
	else if (cameraMode == camFree) {
		hexRender.setCameraPos(freeCamPos.x, freeCamPos.y);
	}

}



/** Whether we're in standard follow-cam mode or not, etc. */
void CHexWorld::setViewMode(TViewMode mode) {
	viewMode = mode;

	if (mode == gameView) {
		hexRendr2.pointCamera(glm::vec3(0, 0, -1));
		hexRendr2.setCameraHeight(15);
	}
	else if (mode == devView) {
		hexRendr2.setCameraPos(glm::vec3(0, -0, 12));
		hexRendr2.setCameraPitch(45);

	}

}

void CHexWorld::adjustZoomScale(float delta) {
	zoomAdjust += (delta > 0) ? -0.1f : 0.1f;
	zoomAdjust = std::max<float>(zoomAdjust, 0.0f);
	zoomScale = 1.0f + (std::pow(zoomAdjust, 0.5f) * 10);
}

/** User dragging map around with mouse, ie, in edit mode.*/
void CHexWorld::onMapDrag() {
	glm::vec3 mouseVec = mouseWorldPos - lastMouseWorldPos;
	//liveLog << "\n" << glm::length(mouseVec);
	cumulativeMapDrag += glm::length(mouseVec);
	if (cumulativeMapDrag < 0.5f)
		return;
	hexRendr2.moveCamera(-mouseVec);
	//cumulativeMapDrag = 0;
	mapDragging = true;
}

void CHexWorld::toggleDirectionGraphics() {
	directionGraphicsOn = !directionGraphicsOn;

	if (directionGraphicsOn) {
		for (auto& entity : map->entities) {
			if (entity->isRobot) {
				auto graphic = std::make_shared<CAvoidGraphic>();
				graphic->entity = entity;
				graphic->pPalette = hexRender.getPalette("mix");
				hexRender.graphics.emplace_back(graphic);

				auto graphic2 = std::make_shared<CDestinationGraphic>();
				graphic2->entity = entity;
				graphic2->pPalette = hexRender.getPalette("blue");
				hexRender.graphics.emplace_back(graphic2);
			}
		}
	}
	else {
		hexRender.graphics.clear();
	}

}

void CHexWorld::followCam(CEntity* ent) {
	if (ent) {
		pFollowCamEnt = ent;
		cameraMode = camFollow;
	}
}

void CHexWorld::freeCam(float x, float y) {
	cameraMode = camFree;
	freeCamPos = glm::vec2(x, y);
}

void CHexWorld::freeCam() {
	cameraMode = camFree;
	freeCamPos = glm::vec2(hexRender.pCamera->getPos().x, hexRender.pCamera->getPos().y);
}

void CHexWorld::fixedCam(float x, float y) {
	cameraMode = camFixed;
	hexRender.setCameraPos(x, y);
}

void CHexWorld::realtimeKeyChecks() {
	if (CWin::keyPressed('T')) {
		startGame();
	}

	if (cameraMode == camFree) {
		if (CWin::keyPressed('W')) moveCamera(glm::vec3{ 0, 1, 0 });
		if (CWin::keyPressed('S')) moveCamera(glm::vec3{ 0, -1, 0 });
		if (CWin::keyPressed('A')) moveCamera(glm::vec3{ -1,0,0 });
		if (CWin::keyPressed('D')) moveCamera(glm::vec3{ 1,0,0 });
	}
}

void CHexWorld::realtimeMouseButtons() {
	if (CWin::mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
		playerObj->onFireKey(true);

	}

}

void CHexWorld::drawReticule() {
	hexRender.drawModelAt(reticule, mouseWorldPos);	
}

/** Remove marked entities from the game. */
void CHexWorld::removeEntities() {
	physics.removeDeadEntities();

	for (auto& gra = hexRender.graphics.begin(); gra != hexRender.graphics.end();) {
		if (gra->get()->entity && gra->get()->entity.get()->toRemove == true)
			gra = hexRender.graphics.erase(gra);
		else
			gra++;
	}

	for (auto& ent = map->entities.begin(); ent != map->entities.end();) {
		if (ent->get()->toRemove == true) {
			if (pBotZero && pBotZero->id == ent->get()->id) {
				pBotZero = nullptr;
			}
			ent = map->entities.erase(ent);
		}
		else
			ent++;
	}

}

void CHexWorld::onPlayerDeath() {
	fixedCam(playerObj->worldPos.x, playerObj->worldPos.y);
	map->removeEntity(playerObj);

	for (auto& entity : map->entities) {
		if (entity->isRobot) {
			((CRobot*)entity.get())->setState(robotWander3);
		}
	}
}

/** TO DO: ultimately this should be automated via a Tig
	config file rather than hardcoded. */
void CHexWorld::initPalettes() {
	hexRender.storePalette("hex", { {0.0f,0.3f,0.0f,1.0f} });
	hexRender.storePalette("largeHex", { { 0.0f,0.0f,0.9f,1.0f}, { 0.5f,0.5f,0.8f,1.0f} });
	hexRender.storePalette("basic", { {1,1,1,1}, {1,1,1,1}, {1,1,1,1}, {0,0,1,1} });
	hexRender.storePalette("gun", { {0, 1, 1.0, 0.45f}, {0,0,0,1}, {1,1,1,1} });
	hexRender.storePalette("armour", { {1, 0, 1.0, 0.45f}, {0,0,0,1}, {1,1,1,1} });
	hexRender.storePalette("explosion", { {1, 1, 0.0,1}, {0,0,1,1}, {1,1,1,1} });
	hexRender.storePalette("mix", { {1,1,1,1}, {1,1,0,1}, {1,0,1,1}, {0,0,1,1} });
	hexRender.storePalette("blue", { {0,0,1,1}, {0.2,0.2,1,1}, {0.3,0.3,1,1}, {0,0,1,1} });
	hexRender.storePalette("test", { {1,0,0,1}, {0,1,0,1}, {0,0,1,1}, {1,1,0,1} });
	hexRender.storePalette("shield", { {0,0,1,0.15f}, {1,0,0,0.15f}, {0,1,0,1}, {1,1,0,1} });


	spawn::pPalettes = &hexRender.palettes;
}