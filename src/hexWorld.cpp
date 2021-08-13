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

CHexWorld::CHexWorld() {
	hexRendr2.init();
	imRendr::setMatrix(&hexRendr2.camera.clipMatrix);
	
	subscribe(&world);

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
	hexRendr2.loadMesh(name, fileName);
}

/** Create a map using the data in the given Tig file. */
void CHexWorld::makeMap(ITigObj* tigMap) {

	//map = mapMaker.makeMap(tigMap);

	map = new CGameHexArray();
	map->init(22,22);

	glm::i32vec2 tL(1);
	glm::i32vec2 bR = { 21,21 };

	for (int y = 0; y < 22; y++) {
		for (int x = 0; x < 22; x++) {
			if (x < tL.x || x >= bR.x || y < tL.y || y >= bR.y)
				map->getHexOffset(x, y).content = 2;
			else
				map->getHexOffset(x, y).content = 1;
			//hexArray->getHexOffset(x, y).fogged = 1.0f;
			//map->setFog(x, y, 1.0f);
		}
	}

	map->setMessageHandlers(); //TO DO: get rid of!

	hexRendr2.setMap(map);
	mapEdit.setMap(map);
}

void CHexWorld::deleteMap() {
	//for (auto entity : map->entities)
	//	unsubscribe(entity.get());
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


	hexRendr2.setMap(map);
	world.setMap(map);
	physics.removeEntities();

	physics.setMap(map);


	//create new player object
	playerObj = new CPlayerObject();
	playerObj->setLineModel("player");
	map->addEntity(TEntity(playerObj), CHex(-6,9,-3));

	world.player = playerObj;

	playerObj->updateViewField(); //TO DO: causes errrors look into pron needs reinventing

	physics.add(playerObj);

	for (auto& entity : map->entities)
		if (entity->isRobot)
			physics.add(entity.get());


	if (hexCursor == NULL)
		createCursorObject();

	map->updateBlocking();

	map->effectsNeedUpdate = true;

	setViewMode(gameView);

	beginNewTurn(); //NB!!! Repeats some of the stuff above


}


void CHexWorld::moveCamera(glm::vec3& direction) {
	float camSpeed = 6.0f * dT;
	glm::vec3 vector = direction *= camSpeed;
	hexRendr2.moveCamera(vector);
}

/** Called when a key is pressed. */
void CHexWorld::onKeyDown(int key, long mod) {




	//temp user interface stuff!

	msg::emit(msg::tmpMsg, key); //temp!

	if (editMode) { //editmode key operations
		if (key == 'R') {
			mapEdit.createRing();
		}

		if (key == GLFW_KEY_ENTER) {
			mapEdit.addEdit();
		}
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

void CHexWorld::onMouseWheel(float delta, int key) {
	if (editMode) {
		if (!mapEdit.mouseWheel(delta,key))
			hexRendr2.dollyCamera(delta);

		return;
	}



	if (key == GLFW_KEY_LEFT_SHIFT) {
		hexRendr2.pitchCamera(delta);
	}
	else {
		if (key == GLFW_KEY_LEFT_CONTROL)
			hexRendr2.dollyCamera(delta);
		else {
			;
		}
	}
}

void CHexWorld::onMouseMove(int x, int y, int key) {
	if (world.paused)
		return;

	mousePos = { x,y };

	calcMouseWorldPos();
}

void CHexWorld::calcMouseWorldPos() {
	auto [mouseHex, mouseWS] = hexRendr2.pickHex(mousePos.x, mousePos.y);
	mouseWorldPos = mouseWS;
	glm::vec3 mouseVec = mouseWorldPos - playerObj->worldPos;
	playerObj->setTargetAngle(glm::orientedAngle(glm::normalize(mouseVec), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1)));

	if (mouseHex != hexCursor->hexPosition) {
		CMouseExitHex msg;
		msg.leavingHex = hexCursor->hexPosition;
		notify(msg);
		onNewMouseHex(mouseHex);
	}
}


void CHexWorld::draw() {

	hexRendr2.drawFloorPlan();

	if (editMode)
		return;


	//for (auto entity : entitiesToDraw)
	for (auto& entity : map->entities)
			entity->draw();



	//hexRendr2.drawSightLine(playerObj->worldPos, mouseWorldPos);

	//temp!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (!world.player->dead) {
		imRendr::setDrawColour({ 1.0f, 1.0f, 1.0f, 0.25f });
		imRendr::drawLine(playerObj->worldPos, mouseWorldPos);
		hexCursor->draw();
	}

	imRendr::drawText(600, 50, "HP: " + std::to_string(world.player->hp));


}

/** Adjust horizontal vs vertical detail of the view. Usually called when the screen size changes. */
void CHexWorld::setAspectRatio(glm::vec2& ratio) {
	hexRendr2.setCameraAspectRatio(ratio);
}


/** Called every frame to get the hex world up to date.*/
void CHexWorld::update(float dT) {

	if (world.paused)
		return;
	this->dT = dT;
	map->setFog(CHex(2, -7, 5), 1.0f);
	//map->setFog(CHex(2, -8, 6), 1.0f);
	map->setFog(CHex(3, -8, 5), 1.0f);

	updateCameraPosition();

	physics.update(dT);

	for (auto& entity : map->entities) {
		entity->update(dT);

	}


	renderer.entityNo = 0;

	if (world.map->entityListDirty)
		physics.removeDeletedEntities();


	world.update(dT);

	if (map->mapUpdated) {
		hexRendr2.setMap(map); //temp to refresh map
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


/////////////public - private devide

void CHexWorld::createCursorObject() {
	hexCursor = new CEntity();
	hexCursor->setLineModel("cursor");
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
	glm::i32vec2 index = map->cubeToIndex(mouseHex);
	coords << " index " << index.x << " " << index.y;
	glm::vec3 worldSpace = cubeToWorldSpace(mouseHex);
	coords << " worldPos " << worldSpace.x << " " << worldSpace.y << " " << worldSpace.z;
	coords << " " << map->getHexCube(mouseHex).content;
	//glm::vec2 screenPos = hexRenderer.worldPosToScreen(worldSpace);
	coords << " wsMouse " << mouseWorldPos.x << " " << mouseWorldPos.y;
	coords << " scrnMouse " << mousePos.x << " " << mousePos.y;
//	CHex test = worldSpaceToHex3(glm::vec3(mouseWorldPos.x, mouseWorldPos.y,0));
//	coords << " rect " << test.x << " " << test.z;


	hexPosLbl->setText(coords.str());

	//hexPosLbl->setText("longer!");

	COnCursorNewHex msg;
	msg.newHex = mouseHex;
	notify(msg);

	if (editMode)
		mapEdit.onNewMouseHex(mouseHex);

}




void CHexWorld::onFireKey(bool pressed) {
	if (editMode)
		mapEdit.addEdit();
	else
		playerObj->onFireKey(pressed);
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




///** Return the CItem or CGroupItem object found at this hex, if any. */
//CGameHexObj* CHexWorld::getItemAt(CHex& position) {
//	for (auto entity : map->entities) {
//		if (entity->hexPosition == position && (entity->isTigClass(tig::CItem)
//			|| entity->isTigClass(tig::CGroupItem)) )
//			return entity.get();
//	}
//	return NULL;
//}

//void CHexWorld::tempGetGroupItem(int itemNo) {
//	CGameHexObj* item = getItemAt(hexCursor->hexPosition);
//	if (item->isTigClass(tig::CGroupItem)) {
//		CGameHexObj*  gotItem = static_cast<CGroupItem*>(item)->removeItem(itemNo);
//		playerObj->takeItem(*gotItem);
//	}
//}



//void CHexWorld::onDropItem(CDropItem& msg) {
//	dropItem(msg.item, msg.location);
//}

//void CHexWorld::onRemoveEntity(CRemoveEntity& msg) {
//	map->deleteEntity(*msg.entity);
//}





/** Make psuedo random dice roll and return the result. */
//void CHexWorld::onDiceRoll(CDiceRoll& msg) {
//	std::uniform_int_distribution<int> d{ 1,msg.die };
//	msg.result =  d(randEngine);
//	if (msg.die2 != 0)
//		msg.result2 = d(randEngine);
//}

//void CHexWorld::onPlayerNewHex(CPlayerNewHex& msg) {
//	//notify(msg);
//	//cursorPath.erase(cursorPath.begin());
//	//hexRenderer.updateFogBuffer();
//}

//void CHexWorld::onActorMovedHex(CActorMovedHex& msg) {
//	notify(msg);
//
//	//alertEntitiesInPlayerFov();
//}




void CHexWorld::updateCameraPosition() {
	if (viewMode == gameView && !world.player->dead && !editMode)
		hexRendr2.followTarget(playerObj->worldPos);
	else
		hexRendr2.attemptScreenScroll(/*mainApp->getMousePos()*/mousePos, dT);

	calcMouseWorldPos();
}

void CHexWorld::beginNewTurn() {

	//world.onscreenRobotAction = false;
	//qps.beginNewTurn();
	//playerObj->onTurnBegin();
	map->updateBlocking();
	//hexRendr2.updateFogBuffer();
	map->effectsNeedUpdate = true;
	//chooseActions();
	cursorPath = map->aStarPath(playerObj->hexPosition, hexCursor->hexPosition);


	///////////////////////////temp end stuff
	//if (playerObj->hexPosition == CHex(13,-4,-9)) {
	//	CSendText msg(combatLog, "\n\nYOU HAVE WON");
	//	send(msg);

	//}
}




///** Update which entities are now visible to the player. */
//void CHexWorld::alertEntitiesInPlayerFov() {
//	for (auto& entity : map->entities) {
//		if (entity.get() == playerObj)
//			continue;
//		bool inView = playerObj->viewField.searchView(entity->hexPosition);
//		entity->playerSight(inView);
//	}
//}

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


