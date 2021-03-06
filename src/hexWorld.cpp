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

CHexWorld::CHexWorld() {
	hexRendr2.init();
	imRendr::setMatrix(&hexRendr2.camera.clipMatrix);
	
//	messageBus.setHandler<CDropItem>(this, &CHexWorld::onDropItem);
	//messageBus.setHandler<CRemoveEntity>(this, &CHexWorld::onRemoveEntity);
	//messageBus.setHandler<CCreateGroupItem>(this, &CHexWorld::onCreateGroupItem);
	messageBus.setHandler<CDiceRoll>(this, &CHexWorld::onDiceRoll);
//	messageBus.setHandler<CPlayerNewHex>(this, &CHexWorld::onPlayerNewHex);
//	messageBus.setHandler<CActorMovedHex>(this, &CHexWorld::onActorMovedHex);


	subscribe(&world);

	hexPosLbl = gui::addLabel("xxx", 10, 10);
	hexPosLbl->setTextColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));


	snd::setVolume(1);

	//msgCB4(2.2f, "hurrah!");

	//int msgId = 66;
	//int msgId2 = 42;
	//int msgId3 = 33;

	msg::attach(msgId, this, &CHexWorld::msgCB3);
	msg::attach(msgId2, this, &CHexWorld::msgCB4);
	msg::attach(msgId3, this, &CHexWorld::msgCB2);

	glm::vec3 v = { 1,2,3 };
	msg::emit(msgId3, v);
	msg::emit(msgId2, 4.2f, "hurrah!", 100);

	msg::remove(msgId2, this);

	msg::emit(msgId2, 4.2f, "hurrah!", 100);
	msg::emit(msgId3, v);

	gWin::createWin("con", 10, 10, 200, 300);
}

void CHexWorld::msgCB(int id) {
	int x = id;
	int b = 0;

}

void CHexWorld::msgCB2( glm::vec3& v) {
	int x = 0;


}

void CHexWorld::msgCB3(float y) {
	float test = y;
	float b = 0;
}

//void CHexWorld::msgCB4(float y,  const std::string& str, int n) {
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
	do {
		map = mapMaker.makeMap(tigMap);
		if (!map->isValidPath(CHex(-13, 5, 8), CHex(13, -4, -9))) {
			delete map;
		}
		else break;
	} while (true);

	map->setMessageHandlers();

	hexRendr2.setMap(map);
}

void CHexWorld::deleteMap() {
	//for (auto entity : map->entities)
	//	unsubscribe(entity.get());
	delete map;
}


/** Required each time we restart. */
void CHexWorld::startGame() {

	//auto begin = std::chrono::high_resolution_clock::now();
	//for (int x = 0; x < 100000; x++) {
	//	worldSpaceToHex3(glm::vec3(x * 3.146f, x * -2.455, 0));
	//}
	//auto end = std::chrono::high_resolution_clock::now();
	//auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

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


	liveLog << "\nAny text here?";

	hexRendr2.setMap(map);
	world.setMap(map);
	physics.removeEntities();

	physics.setMap(map);


	//create new player object
	playerObj = new CPlayerObject();
	playerObj->setLineModel("player");
	map->addEntity(TEntity(playerObj), map->findRandomHex(true));

	//temp brute-force remove nearby robots
	//for (auto& entity : map->entities) {
	//	if (entity->entityType == entRobot && cubeDistance(entity->hexPosition, playerObj->hexPosition) < 8)
	//		map->deleteEntity(*entity);
	//}


	world.player = playerObj;


	//playerObj->setTigObj(vm->getObject("player"));

	//subscribe(playerObj);
	playerObj->updateViewField();
	//alertEntitiesInPlayerFov();

	physics.add(playerObj);

	for (auto& entity : map->entities)
		if (entity->isRobot)
			physics.add(entity.get());


	if (hexCursor == NULL)
		createCursorObject();
	//hexCursor->visibleToPlayer = true;

	CSendText msg(combatLog, "", true);
	send(msg);

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
	hexCursor->draw();

	//for (auto entity : entitiesToDraw)
	for (auto& entity : map->entities)
			entity->draw();


	//for (auto sprite : world.sprites)
	//	sprite->draw();

	//hexRendr2.drawSightLine(playerObj->worldPos, mouseWorldPos);

	//temp!!!!!!!!!!!!!!!!!!!!!!!!!!
	imRendr::setDrawColour({ 1.0f, 1.0f, 1.0f, 0.25f });

	imRendr::drawLine(playerObj->worldPos, mouseWorldPos);

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
}




void CHexWorld::onFireKey(bool pressed) {
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
void CHexWorld::onDiceRoll(CDiceRoll& msg) {
	std::uniform_int_distribution<int> d{ 1,msg.die };
	msg.result =  d(randEngine);
	if (msg.die2 != 0)
		msg.result2 = d(randEngine);
}

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
	if (viewMode == gameView)
		hexRendr2.followTarget(playerObj->worldPos);
	else
		hexRendr2.attemptScreenScroll(/*mainApp->getMousePos()*/mousePos, dT);

	calcMouseWorldPos();
}

void CHexWorld::beginNewTurn() {

	world.onscreenRobotAction = false;
	//qps.beginNewTurn();
	//playerObj->onTurnBegin();
	map->updateBlocking();
	//hexRendr2.updateFogBuffer();
	map->effectsNeedUpdate = true;
	//chooseActions();
	cursorPath = map->aStarPath(playerObj->hexPosition, hexCursor->hexPosition);


	///////////////////////////temp end stuff
	if (playerObj->hexPosition == CHex(13,-4,-9)) {
		CSendText msg(combatLog, "\n\nYOU HAVE WON");
		send(msg);

	}
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


