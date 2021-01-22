#include "hexWorld.h"

#include <string>

#include <glm/gtx/vector_angle.hpp> 

#include "utils/log.h"

#include "gameTextWin.h"

#include "gameState.h"

#include "hexRenderer.h"

CGameHexObj nullGameHexObj;

CHexWorld::CHexWorld() {
	//hexRenderer = &hexRendr2;
	hexRendr2.init();
	CHexObject::setHexRenderer(&hexRendr2);
	
	messageBus.setHandler<CDropItem>(this, &CHexWorld::onDropItem);
	messageBus.setHandler<CRemoveEntity>(this, &CHexWorld::onRemoveEntity);
	messageBus.setHandler<CCreateGroupItem>(this, &CHexWorld::onCreateGroupItem);
	messageBus.setHandler<CDiceRoll>(this, &CHexWorld::onDiceRoll);
	messageBus.setHandler<CPlayerNewHex>(this, &CHexWorld::onPlayerNewHex);
	messageBus.setHandler<CActorMovedHex>(this, &CHexWorld::onActorMovedHex);


	subscribe(&world);

	hexPosLbl = new CGUIlabel2(10, 10, 750, 30);
	hexPosLbl->setTextColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
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
	for (auto entity : map->entities)
		unsubscribe(entity.get());
	delete map;
}


/** Required each time we restart. */
void CHexWorld::startGame() {
	hexRendr2.setMap(map);
	world.setMap(map);


	

	
	//create new player object
	playerObj = new CPlayerObject();
	playerObj->setLineModel("player");
	map->addEntity(TEntity(playerObj), CHex(0, -8, 8));

	world.player = playerObj;


	playerObj->setTigObj(vm->getObject("player"));
	//map->entities.push_back(playerObj);
	subscribe(playerObj);
	playerObj->updateViewField();
	alertEntitiesInPlayerFov();



	entitiesToDraw.assign(map->entities.rbegin(),map->entities.rend());

	world.setTurnPhase(playerPhase);

	if (hexCursor == NULL)
		createCursorObject();
	hexCursor->visibleToPlayer = true;

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
	if (key == 'I')
		playerObj->showInventory();

	if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
		int item = key - GLFW_KEY_1;
		if (mod == GLFW_MOD_SHIFT) {
			tempGetGroupItem(item);
		}
		else if (mod == GLFW_MOD_CONTROL)
			playerObj->equipItem(item);
		else
			playerObj->dropItem(item);
	}

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
	mousePos = { x,y };

	auto [mouseHex, mouseWS ] = hexRendr2.pickHex(x, y);
	mouseWorldPos = mouseWS;
	glm::vec3 mouseVec = mouseWorldPos - playerObj->worldPos;
	playerObj->setTargetAngle(glm::orientedAngle(glm::normalize(mouseVec), glm::vec3(1, 0, 0), glm::vec3(0, 0, -1)));


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


	for (auto sprite : world.sprites)
		sprite->draw();

	//hexRendr2.drawSightLine(playerObj->worldPos, mouseWorldPos);

}

/** Adjust horizontal vs vertical detail of the view. Usually called when the screen size changes. */
void CHexWorld::setAspectRatio(glm::vec2& ratio) {
	hexRendr2.setCameraAspectRatio(ratio);
}


/** Called every frame to get the hex world up to date.*/
void CHexWorld::update(float dT) {
	this->dT = dT;
	map->setFog(CHex(2, -7, 5), 1.0f);
	//map->setFog(CHex(2, -8, 6), 1.0f);
	map->setFog(CHex(3, -8, 5), 1.0f);

	updateCameraPosition();

	for (auto& entity : map->entities) {
		entity->update(dT);
	}

	for (auto& entity : world.sprites) {
		entity->update(dT);
	}

	world.update(dT);
}



/** User has triggered the ctrl left mouse  event. */
void CHexWorld::onCtrlLMouse() {
	
}


/** Player has pressed the enter key. */
void CHexWorld::enterKeyDown() {
	deleteMap();
	entitiesToDraw.clear();

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
	hexCursor = new CGameHexObj();
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
	hexPosLbl->setText(coords.str());

	COnCursorNewHex msg;
	msg.newHex = mouseHex;
	notify(msg);
}


/** Begin a player right-click action. */
void CHexWorld::rightClick() {

}

/** Begin a player left-click action. */
void CHexWorld::leftClick() {
	return;

	if (world.getTurnPhase() != playerPhase)
		return;

	CGameHexObj* obj = getPrimaryObjectAt(hexCursor->hexPosition);

	if (powerMode) {
		obj->leftClickPowerMode();
		return;
	}

	if (obj != &nullGameHexObj) {
		obj->leftClick();
		return;
	}

	return;
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


/** Cope with the player dropping an item. */
void CHexWorld::dropItem(CGameHexObj* item, CHex& location) {
	CGameHexObj* existingItem = getItemAt(location);
	if (existingItem) {
		existingItem->droppedOnBy(*item);
		return;
	}

	item->setPosition(location);
	//map->entities.push_back(TEntity(item));
	map->addEntity(TEntity(item), location);
}

CGroupItem* CHexWorld::createGroupItem() {
	CGroupItem* groupItem = new CGroupItem();
	groupItem->setLineModel("test");
	groupItem->setTigObj(vm->getObject(tig::CGroupItem));
	map->entities.push_back(TEntity(groupItem));
	return groupItem;
}

/** Return the CItem or CGroupItem object found at this hex, if any. */
CGameHexObj* CHexWorld::getItemAt(CHex& position) {
	for (auto entity : map->entities) {
		if (entity->hexPosition == position && (entity->isTigClass(tig::CItem)
			|| entity->isTigClass(tig::CGroupItem)) )
			return entity.get();
	}
	return NULL;
}

void CHexWorld::tempGetGroupItem(int itemNo) {
	CGameHexObj* item = getItemAt(hexCursor->hexPosition);
	if (item->isTigClass(tig::CGroupItem)) {
		CGameHexObj*  gotItem = static_cast<CGroupItem*>(item)->removeItem(itemNo);
		playerObj->takeItem(*gotItem);
	}
}



void CHexWorld::onDropItem(CDropItem& msg) {
	dropItem(msg.item, msg.location);
}

void CHexWorld::onRemoveEntity(CRemoveEntity& msg) {
	map->removeEntity(msg.entity);
}

void CHexWorld::onCreateGroupItem(CCreateGroupItem& msg) {
	CGroupItem* groupItem = createGroupItem();
	groupItem->items.push_back(msg.item1);
	groupItem->items.push_back(msg.item2);
//	map->removeFromMap(msg.item2);
	map->removeEntity(msg.item2);
	groupItem->setPosition(playerObj->hexPosition);
}



/** Make psuedo random dice roll and return the result. */
void CHexWorld::onDiceRoll(CDiceRoll& msg) {
	std::uniform_int_distribution<int> d{ 1,msg.die };
	msg.result =  d(randEngine);
	if (msg.die2 != 0)
		msg.result2 = d(randEngine);
}

void CHexWorld::onPlayerNewHex(CPlayerNewHex& msg) {
	notify(msg);
	//cursorPath.erase(cursorPath.begin());
	//hexRenderer.updateFogBuffer();
}

void CHexWorld::onActorMovedHex(CActorMovedHex& msg) {
	notify(msg);

	alertEntitiesInPlayerFov();
}


/** Return the highest priority object at this hex. ie, the one if 
	clicked-on we would want to respond. */
CGameHexObj* CHexWorld::getPrimaryObjectAt(CHex& hex) {
	TRange entities = map->getEntitiesAt(hex);

	for (auto it = entities.first; it != entities.second; it++) {
		if (it->second->isActor())
			return it->second;
	}

	for (auto it = entities.first; it != entities.second; it++) {
		if (it->second->isTigClass(tig::CItem))
			return it->second;
	}

	for (auto it = entities.first; it != entities.second; it++) {
		if (it->second->isTigClass(tig::CDoor))
			return it->second;
	}

	return &nullGameHexObj;
}

void CHexWorld::updateCameraPosition() {
	if (viewMode == gameView)
		hexRendr2.followTarget(playerObj->worldPos);
	else
		hexRendr2.attemptScreenScroll(/*mainApp->getMousePos()*/mousePos, dT);
}

void CHexWorld::beginNewTurn() {
	world.setTurnPhase(playerPhase);
	world.onscreenRobotAction = false;
	qps.beginNewTurn();
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

		world.setTurnPhase(playerDeadPhase);
	}
}




/** Update which entities are now visible to the player. */
void CHexWorld::alertEntitiesInPlayerFov() {
	for (auto& entity : map->entities) {
		if (entity.get() == playerObj)
			continue;
		bool inView = playerObj->viewField.searchView(entity->hexPosition);
		entity->playerSight(inView);
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


