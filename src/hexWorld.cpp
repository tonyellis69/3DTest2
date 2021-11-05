#include "hexWorld.h"

#include <string>

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

#include "tigExport.h"
 
#include "UI/fonts.h" //temp test

#include "renderer/imRendr/imRendr.h"

#include "GLFW/glfw3.h"

#include "spawner.h"

CHexWorld::CHexWorld() {

	spawn::setCallback(this, &CHexWorld::onSpawn);

	hexRendr2.init();
	imRendr::setMatrix(&hexRendr2.camera.clipMatrix);
	
	//subscribe(&game);

	hexPosLbl = gui::addLabel("xxx", 10, 10);
	hexPosLbl->setTextColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	snd::setVolume(1);


	msg::attach(msgId2, this, &CHexWorld::msgCB4);
	msg::emit(msgId2, 4.2f, "hurrah!", 100);
	msg::remove(msgId2, this);

	gWin::createWin("con", 10, 10, 200, 300);
}


void CHexWorld::msgCB4(float y, const char* str, int n) {
	float test = y;
	float b = 0;
}

void CHexWorld::setVM(Ivm* pVM) {
	vm = pVM;
	mapMaker.attachVM(pVM);
	setTigObj(vm->getObject(tig::IHexWorld)); 
	CGameTextWin::pVM = pVM;
}

/**	Load a multipart mesh for storage under the given name. */
void CHexWorld::addMesh(const std::string& name, const std::string& fileName) {
	CImporter importer;
	importer.loadFile(fileName);
	spawn::meshBufs[name] = importer.getSingleMesh().exportToBuffer();

	spawn::modelBufs[name] = importer.getMeshNodes();
	spawn::modelBufs[name].name = name;
	
}

/** Create a map using the data in the given Tig file. */
void CHexWorld::makeMap(ITigObj* tigMap) {

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

	gWin::setFont("con", "mainFnt");

	gWin::addText("con", "1Here is some \\bbold\\b text.");
	gWin::addText("con", "\n2Here is some more text.");
	gWin::addText("con", "\n3Here is some more text.");
	gWin::setColour("con", glm::vec4{ 1,1,0,1 });
	gWin::addText("con", "\n4Here is some more text.");
	gWin::addText("con", "\n5Here is some more text.");
	gWin::addText("con", "\n6Here is some more text.");
	gWin::addText("con", "\n7Here is some more text.");
	gWin::addText("con", "\nAnd here is some more text");
	//gWin::addText("con", "\n9");


	hexRendr2.setMap(map->getHexArray());
	game.setMap(map);
	physics.removeEntities();

	physics.setMap(map->getHexArray());




	//spawn::player("player", cubeToWorldSpace(CHex(-6,9-3)));

	mapEdit.load();

	if (hexCursor == NULL)
		createCursorObject();

	map->getHexArray()->effectsNeedUpdate = true;

	setViewMode(gameView);

	map->getHexArray()->effectsNeedUpdate = true; //old code! Replace


}


void CHexWorld::moveCamera(glm::vec3& direction) {
	float camSpeed = 6.0f * dT;
	glm::vec3 vector = direction *= camSpeed;
	hexRendr2.moveCamera(vector);
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
			physics.removeEntities();
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
 
}

void CHexWorld::onKeyUp(int key, long mod) {
	if (editMode) {
		mapEdit.onEntityMode(false);
		mapEdit.onShapeMode(false);
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
			hexRendr2.dollyCamera(delta * zoomScale);
			adjustZoomScale(delta);
		}
		return;
	}



	if (key == GLFW_KEY_LEFT_SHIFT) {
		hexRendr2.pitchCamera(delta);
	}
	else {
		if (key == GLFW_KEY_LEFT_CONTROL) {
			hexRendr2.dollyCamera(delta * zoomScale);
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
}

void CHexWorld::calcMouseWorldPos() {
	auto [mouseHex, mouseWS] = hexRendr2.pickHex(mousePos.x, mousePos.y);
	lastMouseWorldPos = mouseWorldPos;
	mouseWorldPos = mouseWS;
	glm::vec3 mouseVec = mouseWorldPos - playerObj->worldPos;
	playerObj->setUpperBodyRotation(glm::orientedAngle(glm::normalize(mouseVec), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1)));

	if (mouseHex != hexCursor->hexPosition) {
		CMouseExitHex msg;
		msg.leavingHex = hexCursor->hexPosition;
		onNewMouseHex(mouseHex);
	}
}


void CHexWorld::draw() {
	hexRendr2.drawFloorPlan();

	for (auto& entity : map->entities) {
		if (entity->live)
			entity->draw();
	}

	if (editMode) {
		if (mapEdit.entityMode) {
			imRendr::drawText(50, 40, mapEdit.currentEntStr);
		}
		else if (mapEdit.shapeMode) {
			imRendr::drawText(50, 40, mapEdit.currentShapeStr);
		}

		return;
	}


	//hexRendr2.drawSightLine(playerObj->worldPos, mouseWorldPos);

	//temp!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (!game.player->dead) {
		imRendr::setDrawColour({ 1.0f, 1.0f, 1.0f, 0.25f });
		imRendr::drawLine(playerObj->worldPos, mouseWorldPos);
		hexCursor->draw();
	}

	imRendr::drawText(600, 50, "HP: " + std::to_string(game.player->hp));


}

/** Adjust horizontal vs vertical detail of the view. Usually called when the screen size changes. */
void CHexWorld::setAspectRatio(glm::vec2& ratio) {
	hexRendr2.setCameraAspectRatio(ratio);
}


/** Called every frame to get the hex world up to date.*/
void CHexWorld::update(float dT) {

	if (game.paused)
		return;

	this->dT = dT;


	updateCameraPosition();

	physics.update(dT);

	if (!editMode)
		for (int n = 0; n < map->entities.size(); n++) {
			auto entity = map->entities[n];
			if (entity->live)
				map->entities[n]->update(dT);
		}


	if (game.map->entitiesToDelete)
		physics.removeDeletedEntities();

	//TO DO: this should come to replace above
	if (game.map->entitiesToKill) {
		physics.removeDeadEntities();
		game.map->entitiesToKill = false;
	}


	game.update(dT);

	if (map->mapUpdated) {
		hexRendr2.setMap(map->getHexArray()); //temp to refresh map
		map->mapUpdated = false;
	}

}



/** User has triggered the ctrl left mouse  event. */
void CHexWorld::onCtrlLMouse() {
	
}


/** Player has pressed the enter key. */
void CHexWorld::enterKeyDown() {
	deleteMap();
	//entitiesToDraw.clear();

	makeMap(vm->getObject("testRoom"));
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
		gWin::hideWin("con");
	}
	else {
		gWin::showWin("con");
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


void CHexWorld::onSpawn(const std::string& name, TEntity entity) {
	if (name == "melee bot" || name == "shooter bot") {
		physics.add(entity.get());
	}

	if (name == "player") {
		game.player = (CPlayerObject*) entity.get();
		physics.add(entity.get());
		playerObj = (CPlayerObject*)entity.get();
		game.player = playerObj;
	}

	map->entities.push_back(entity);

}


/////////////public - private devide

void CHexWorld::createCursorObject() {
	hexCursor = new CEntity();
	hexCursor->setBoundingRadius();
	hexCursor->lineModel.buffer2 = &spawn::meshBufs["cursor"];
	hexCursor->lineModel.model = spawn::modelBufs["cursor"];
	hexCursor->lineModel.setColourR(glm::vec4(0.3, 1, 0.3, 1));
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
	else
		playerObj->onFireKey(stillPressed);
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
	std::string strParam;
	switch (memberId) {
	case tig::msgFn : 
		strParam = vm->getParamStr(0);
		liveLog << strParam; break;
	case tig::popupWin :
		strParam = vm->getParamStr(0);
		;// mothballing this, use messaging popupMsg(strParam); break;
	}
	return 1;
}


void CHexWorld::updateCameraPosition() {
	if (viewMode == gameView && !editMode && !game.player->dead )
		hexRendr2.followTarget(playerObj->worldPos);
	//else
		//hexRendr2.attemptScreenScroll(mousePos, dT);

	calcMouseWorldPos();
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

void CHexWorld::adjustZoomScale(float delta)
{
	zoomAdjust += (delta > 0) ? -0.1f : 0.1f;
	zoomAdjust = std::max(zoomAdjust, 0.0f);
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


