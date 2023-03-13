#include "hexWorld.h"

#include <string>
#include <algorithm>

#include <glm/gtx/vector_angle.hpp> 

#include "UI\gui.h"

#include "utils/log.h"
#include "gameTextWin.h"
#include "gameState.h"
//#include "hexRenderer.h"

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

#include  "importer/importer.h"

CHexWorld::CHexWorld() {
	game.paused = true;

	hexRender.init();

	imRendr::setMatrix(&hexRender.camera.clipMatrix);
	
	lis::subscribe<CGUIevent>(this);
	lis::subscribe<CGameEvent>(this);
	lis::subscribe<CPhysicsEvent>(this);

	hexPosLbl = gui::addLabel("xxx", 10, 10);
	hexPosLbl->setTextColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	snd::setVolume(1);

	gWin::initWindows();

	initPalettes();

}

void CHexWorld::onEvent(CGUIevent& e) {
	if (procTestMode) {
		if (e.type == eMouseWheel) {
			levelGen.resize(int(e.f2));
			makeMap();
			startProcTest();
		}
		if (e.type == eKeyDown) {
			if (e.i1 == 'L') {
				levelGen.subdivide();
				makeMap();
				startProcTest();
			}
			if (e.i1 == 'R') {
				levelGen.quadRemovals();
				makeMap();
				startProcTest();

			}
		}

	}



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

void CHexWorld::onEvent(CPhysicsEvent& e) {
	if (e.action == physAdd)
		physics.add(e.entity);
	else if (e.action == physRemove)
		physics.remove(e.entity);
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


void CHexWorld::makeMap() {
	//makemap old
	//level = new CLevel();
	//level->init(100, 40);
	auto level = std::make_unique<CLevel>();
	level->init(100, 40);


	for (int y = 0; y < 40; y++) {
		for (int x = 0; x < 100; x++) {
			level->getHexOffset(x, y).content = emptyHex;
		}
	}

	mapEdit.setMap(level.get());
	game.setLevel(std::move(level)); //added!

	//makemap new
	//auto level = levelGen.makeLevel();
	//game.setLevel(std::move(level));
}

void CHexWorld::deleteMap() {
	//delete level;
}


/** Required each time we restart. */
void CHexWorld::startGame() {

	//game.setLevel(level); //old

	physics.clearEntities();
	physics.setMap(game.level->getHexArray());

	mapEdit.load();
	//!!!!!!!!!Previous point where map file was loaded

	hexRender.loadMap(game.level->getHexArray());
	prepMapEntities();

	if (hexCursor == NULL)
		createCursorObject();

	setViewMode(gameView);

	gWin::pInv->refresh();

	game.paused = false;

	followCam(playerObj);
	//freeCam(-76, 15);
//	toggleDirectionGraphics();
	//game.slowed = true;

	pBotZero = nullptr;
	for (auto& entity : game.entities) {
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

void CHexWorld::startProcTest() {
	procTestMode = true;
	hexRender.loadMap(game.level->getHexArray());
	if (hexCursor == NULL)
		createCursorObject();
	if (hexRender.camera.getPos() == glm::vec3(0))
		setViewMode(gameView);
	else
		setViewMode(keepView);
	game.paused = false;
	freeCam();
	CWin::fullScreen();
	zoom2fit = true;
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
		/*killMeOldHexRenderer.toggleFollowCam();
		killMeOldHexRenderer.followTarget(playerObj->getPos());*/
	}

	if (key == 'R') {
		//killMeOldHexRenderer.hexLineShader->recompile();
		//killMeOldHexRenderer.lineShader->recompile();
		//killMeOldHexRenderer.hexSolidShader->recompile();
		//killMeOldHexRenderer.visibilityShader->recompile();
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
			if (hexRender.dollyCamera(delta * zoomScale))
				adjustZoomScale(delta);
		}
		return;
	}



	if (key == GLFW_KEY_LEFT_SHIFT) {
		hexRender.pitchCamera(delta);
	}
	else {
		if (key == GLFW_KEY_LEFT_CONTROL) {
			if (hexRender.dollyCamera(delta * zoomScale) )
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

	CHex lastMouseHex = hexCursor->transform->hexPosition;

	calcMouseWorldPos();
	
	if (key == GLFW_MOUSE_BUTTON_RIGHT && 
		hexCursor->transform->hexPosition != lastMouseHex &&  editMode)
		mapEdit.onRightDrag();

	if (key == GLFW_MOUSE_BUTTON_LEFT && editMode) {
		onMapDrag();
	}

	if (editMode)
		mapEdit.onMouseMove(mouseWorldPos);
}

void CHexWorld::calcMouseWorldPos() {
	glm::vec3 mouseWS = hexRender.screenToWS(mousePos.x, mousePos.y);
	CHex mouseHex = worldSpaceToHex(mouseWS);
	lastMouseWorldPos = mouseWorldPos;
	mouseWorldPos = mouseWS;
	if (playerObj) {
		glm::vec3 mouseVec = mouseWorldPos - playerObj->getPos();
		playerObj->setMouseDir(glm::normalize(mouseVec));
	} //needed for playerObj orientation etc

	if (mouseHex != hexCursor->transform->hexPosition) {
		CMouseExitHex msg;
		msg.leavingHex = hexCursor->transform->hexPosition;
		onNewMouseHex(mouseHex);
	}
}


void CHexWorld::draw() {
	hexRender.drawMap();

	hexRender.resetDrawLists();

	for (auto& entity : game.entities) {
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

	if (game.player && !game.player->dead) {
		//imRendr::setDrawColour({ 1.0f, 1.0f, 1.0f, 1.0f });
		//imRendr::drawLine(playerObj->worldPos, mouseWorldPos);
		//hexCursor->draw();
		drawReticule();
	}

	if (game.player) {
		int sh = ((CShieldComponent*)game.player->shield->item.get())->hp;

		imRendr::drawText(600, 50, "HP: " + std::to_string(game.player->hp)
			+ " Shield: " + std::to_string(sh)

		);
	}

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

	if (pBotZero && pBotZero->deleteMe == false)
		imRendr::drawText(600, 80, pBotZero->diagnostic);

}

/** Adjust horizontal vs vertical detail of the view. Usually called when the screen size changes. */
void CHexWorld::setAspectRatio(glm::vec2& ratio) {
	float defFov = glm::radians(45.0f);
	float defaultScreenH = 800.0f;
	float newFov = asin(sin(defFov / 2.0f) * ratio.y / defaultScreenH) * 2.0f;
	hexRender.setCameraAspectRatio(ratio,newFov);
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
		for (int n = 0; n < game.entities.size(); n++) {
			auto& entity = game.entities[n];
			if (entity->live)
				game.entities[n]->update(dT);
		}




	//if (game.entitiesToDelete)
	//	physics.removeDeletedEntities();

	//TO DO: this should come to replace above
	if (game.entitiesToKill) {
		removeDeadEntities();
	}


	game.update(dT);

	if (game.level->mapUpdated) {
		//hexRendr2.setMap(level->getHexArray()); //temp to refresh map
		hexRender.loadMap(game.level->getHexArray());
		game.level->mapUpdated = false;
	}

	gWin::update(dT);

	for (auto& graphic : hexRender.graphics) {
		graphic->update(dT);
	}


	realtimeKeyChecks();
	realtimeMouseButtons();

	//removeEntities();

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
		mapEdit.setMap(game.level.get());
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

	for (auto& entity : game.entities) {
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
	hexCursor->setPosition(glm::vec3(0, 0, 0));
}


/** Respond to mouse cursor moving to a new hex. */
void CHexWorld::onNewMouseHex(CHex& mouseHex) {
	hexCursor->setPosition(cubeToWorldSpace(mouseHex));


	/*if (!lineOfSight)
		cursorPath = level->aStarPath(playerObj->hexPosition, hexCursor->hexPosition, true);
	else
		cursorPath = *hexLine2(playerObj->hexPosition, hexCursor->hexPosition);*/


	std::stringstream coords; coords << "cube " << mouseHex.x << ", " << mouseHex.y << ", " << mouseHex.z;
	glm::i32vec2 offset = cubeToOffset(mouseHex);
	coords << "  offset " << offset.x << ", " << offset.y;
	glm::i32vec2 index = game.level->getHexArray()->cubeToIndex(mouseHex);
	coords << " index " << index.x << " " << index.y;
	glm::vec3 worldSpace = cubeToWorldSpace(mouseHex);
	coords << " worldPos " << worldSpace.x << " " << worldSpace.y << " " << worldSpace.z;
	coords << " " << game.level->getHexArray()->getHexCube(mouseHex).content;
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
	if (cameraMode == camFollow) {
		if (pFollowCamEnt) {
			 hexRender.setCameraPos(pFollowCamEnt->getPos().x, pFollowCamEnt->getPos().y);
		}
	}
	else if (cameraMode == camFree) {
		hexRender.setCameraPos(freeCamPos.x, freeCamPos.y);
	}

	if (zoom2fit)
		zoomToFit();

}



/** Whether we're in standard follow-cam mode or not, etc. */
void CHexWorld::setViewMode(TViewMode mode) {
	viewMode = mode;

	if (mode == gameView) {
		hexRender.pointCamera(glm::vec3(0, 0, -1));
		hexRender.setCameraHeight(15);
	}
	else if (mode == devView) {
		hexRender.setCameraPos(glm::vec3(0, -0, 12));
		hexRender.setCameraPitch(45);
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
	hexRender.moveCamera(-mouseVec);
	//cumulativeMapDrag = 0;
	mapDragging = true;
}

void CHexWorld::toggleDirectionGraphics() {
	directionGraphicsOn = !directionGraphicsOn;

	if (directionGraphicsOn) {
		for (auto& entity : game.entities) {
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
	freeCamPos = glm::vec2(hexRender.camera.getPos().x, hexRender.camera.getPos().y);
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

void CHexWorld::removeDeadEntities() {
	physics.removeDeadEntities();

	for (auto& ent = game.entities.begin(); ent != game.entities.end();) {
		if (ent->get()->deleteMe)
			ent = game.entities.erase(ent);
		else
			ent++;
	}


	game.entitiesToKill = false;
}

///** Remove marked entities from the game. */
//void CHexWorld::removeEntities() {
//	physics.removeDeadEntities();
//
//	for (auto& gra = hexRender.graphics.begin(); gra != hexRender.graphics.end();) {
//		if (gra->get()->entity && gra->get()->entity.get()->toRemove == true)
//			gra = hexRender.graphics.erase(gra);
//		else
//			gra++;
//	}
//
//	for (auto& ent = game.entities.begin(); ent != game.entities.end();) {
//		if (ent->get()->toRemove == true) {
//			if (pBotZero && pBotZero->id == ent->get()->id) {
//				pBotZero = nullptr;
//			}
//			ent = game.entities.erase(ent);
//		}
//		else
//			ent++;
//	}
//
//}

void CHexWorld::onPlayerDeath() {
	fixedCam(playerObj->getPos().x, playerObj->getPos().y);
	//level->removeEntity(playerObj);
	playerObj->setPosition(glm::vec3(0));

	for (auto& entity : game.entities) {
		if (entity->isRobot) {
			//((CRobot*)entity.get())->setState(robotWander3);
			entity->ai = std::make_shared<CRoboWander>((CRobot*)entity.get());
		}
	}
}

/** Progressively zoom while the map doesn't fit the screen. */
void CHexWorld::zoomToFit() {
	glm::vec3 gridTL = abs(cubeToWorldSpace(game.level->indexToCube( glm::i32vec2{ 0,0 })));
	glm::vec3 gridBR = abs(cubeToWorldSpace(game.level->indexToCube(game.level->getGridSize())));

	glm::i32vec2 scnSize = hexRender.getScreenSize();
	glm::vec3 scnTL = abs(hexRender.screenToWS(0, 0));
	glm::vec3 scnBR = abs(hexRender.screenToWS(scnSize.x, scnSize.y));

	glm::vec3 margin(2, 2, 0);

	if (glm::any(glm::greaterThan(gridTL + margin, scnTL))) {
		hexRender.dollyCamera(-1.0f);
	} 
	else	if (glm::all(glm::lessThanEqual(gridTL , scnTL - margin * 1.5f))) {
		hexRender.dollyCamera(1.0f);
	}
	else
		zoom2fit = false;

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