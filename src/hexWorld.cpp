#include "hexWorld.h"

#include <string>

#include "utils/log.h"

#include "gameTextWin.h"

CHexWorld::CHexWorld() {
	CHexObject::setHexRenderer(&hexRenderer);
	CGridObj::setHexRenderer(&hexRenderer);
	
	mapMaker.entities = &entities;

	messageBus.setHandler<CAddActor>(this, &CHexWorld::onAddActor);
	messageBus.setHandler<CShootAt>(this, &CHexWorld::onShootAt);
	messageBus.setHandler<CDropItem>(this, &CHexWorld::onDropItem);
	messageBus.setHandler<CRemoveEntity>(this, &CHexWorld::onRemoveEntity);
	messageBus.setHandler<CCreateGroupItem>(this, &CHexWorld::onCreateGroupItem);

}

/** Provide a pointer to the game app to check for mouse input, etc. */
void CHexWorld::setMainApp(IMainApp* pApp) {
	//TO DO: aim to replace with messaging!!!
	this->mainApp = pApp;	
}

void CHexWorld::setVM(Ivm* pVM) {
	vm = pVM;
	mapMaker.attachVM(pVM);
	setTigObj(vm->getObject(tig::IHexWorld)); 
	CGameTextWin::pVM = pVM;
}

/**	Load a multipart mesh for storage under the given name. */
void CHexWorld::addMesh(const std::string& name, const std::string& fileName) {
	hexRenderer.loadMesh(name, fileName);
}

/** Create a map using the data in the given Tig file. */
void CHexWorld::makeMap(ITigObj* tigMap) {
	mapMaker.attachMapObject(tigMap);
	map = mapMaker.createMap();

	map.setMessageHandlers();
}

/** Late set-up stuff. */
void CHexWorld::start() {
	turnPhase = actionPhase;// chooseActionPhase;

	temporaryCreateHexObjects();

	tempPopulateMap();

	//TO DO: another kludge: find a way to subscribe entities on creation
	for (auto& entity : entities) {
		subscribe(entity);
	}

	hexRenderer.setMap(&map);

	map.setEntityList(&entities);

	hexRenderer.start();	

	hexPosLbl = new CGUIlabel2(10, 10, 510, 30);
	//hexPosLbl->anchorBottom = 20;
	hexPosLbl->setTextColour(glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f ));
	mainApp->addGameWindow(hexPosLbl);
}



void CHexWorld::moveCamera(glm::vec3& direction) {
	float camSpeed = 6.0f * dT;
	glm::vec3 vector = direction *= camSpeed;
	hexRenderer.moveCamera(vector);
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
		hexRenderer.toggleFollowCam();
		hexRenderer.followTarget(playerObj->worldPos);
	}

}



void CHexWorld::onMouseWheel(float delta) {
	if (mainApp->hexKeyNowCallback(GLFW_KEY_LEFT_SHIFT)) {
		hexRenderer.pitchCamera(delta);
	}
	else {
		if (mainApp->hexKeyNowCallback(GLFW_KEY_LEFT_CONTROL))
			hexRenderer.dollyCamera(delta);
		else {
			;
		}
	}
}

void CHexWorld::onMouseMove(int x, int y, int key) {
	mousePos = { x,y };
	CHex mouseHex = hexRenderer.pickHex(x, y);
	if (mouseHex != hexCursor->hexPosition) {
		CMouseExitHex msg;
		msg.leavingHex = hexCursor->hexPosition;
		notify(msg);
		onNewMouseHex(mouseHex);
	}
}



void CHexWorld::draw() {
	if (turnPhase == playerChoosePhase && !powerMode) {
		hexRenderer.draw();
		hexCursor->draw();
		hexRenderer.drawPath(&cursorPath, glm::vec4{ 0.6, 0.4, 1, 0.1f }, glm::vec4{ 0.6, 0.4, 1, 0.75f });
	}
	else
		hexRenderer.draw();

	for (auto entity : entities)
		entity->draw();
	for (auto gridObj : gridObjects)
		gridObj->draw();
}

/** Adjust horizontal vs vertical detail of the view. Usually called when the screen size changes. */
void CHexWorld::setAspectRatio(glm::vec2& ratio) {
	hexRenderer.setCameraAspectRatio(ratio);
}


/** Called every frame to get the hex world up to date.*/
void CHexWorld::update(float dT) {
	this->dT = dT;

	removeDeletedEntities();

	if (hexRenderer.following())
		hexRenderer.followTarget(playerObj->worldPos);
	else
		hexRenderer.attemptScreenScroll(mainApp->getMousePos(),dT);

	for (auto entity : entities)
		entity->frameUpdate(dT);
	
	resolvingGridObjActions();

	if (turnPhase == actionPhase) { 	

		if (resolvingSerialActions())
			return;

		if (resolvingSimulActions()) {
			return;
		}

		//end of action phase
		playerObj->onTurnBegin();
		map.updateBlocking();
		chooseActions();

	}

}


/** User has triggered the ctrl left mouse  event. */
void CHexWorld::onCtrlLMouse() {
	
}

/** User has released set-defence key. */
void CHexWorld::onCtrlRelease() {
	//fireablePanel->onRelease();
}

/** Player has pressed the power mode on/off key. */
void CHexWorld::powerModeToggle() {
	//TO DO: call func on QPS
	//for now, just do this
	powerMode = !powerMode;
	CSysMsg msg(powerMode);
	send(msg);
}



/////////////public - private devide

/** Plot a navigable path from start to end on the current map. */
THexList CHexWorld::calcPath(CHex& start, CHex& end) {
	return map.aStarPath(start,end);
}



/** TO DO: temporary.This should not be hard-coded, but spun from a level-maker. */
void CHexWorld::temporaryCreateHexObjects() {
	playerObj = new CPlayerObject();
	playerObj->setLineModel("player");

//	map.add(playerObj, CHex(-5, -1, 6));
	map.add(playerObj, CHex(-2, 2, 0));

	playerObj->setTigObj(vm->getObject("player"));


	entities.push_back(playerObj);

	hexCursor = new CGameHexObj();
	hexCursor->setLineModel("cursor");
	hexCursor->setPosition(CHex(0, 0, 0));


}


/** Respend to mouse cursor moving to a new hex. */
void CHexWorld::onNewMouseHex(CHex& mouseHex) {
	hexCursor->setPosition(mouseHex);

	cursorPath = calcPath(playerObj->hexPosition, hexCursor->hexPosition);


	std::stringstream coords; coords << "cube " << mouseHex.x << ", " << mouseHex.y << ", " << mouseHex.z;
	glm::i32vec2 offset = cubeToOffset(mouseHex);
	coords << "  offset " << offset.x << ", " << offset.y;
	glm::i32vec2 index = map.cubeToIndex(mouseHex);
	coords << " index " << index.x << " " << index.y;
	glm::vec3 worldSpace = cubeToWorldSpace(mouseHex);
	coords << " worldPos " << worldSpace.x << " " << worldSpace.y << " " << worldSpace.z;
	coords << " " << map.getHexCube(mouseHex).content;
	//glm::vec2 screenPos = hexRenderer.worldPosToScreen(worldSpace);
	//coords << " " << screenPos.x << " " << screenPos.y;
	hexPosLbl->setText(coords.str());

	 

	COnNewHex msg;
	msg.newHex = mouseHex;
	notify(msg);
}


/** Return the player object's current travel path. */
THexList* CHexWorld::getCursorPath() {
	return &cursorPath;
}

CGameHexObj* CHexWorld::getPlayerObj() {
	return playerObj;
}

CHexObject* CHexWorld::getCursorObj() {
	return hexCursor;
}

/** All entities choose their action for the coming turn. */
void CHexWorld::chooseActions() {
	currentSerialActor = NULL;
	for (auto entity : entities) {
		if (entity->isActor())
			static_cast<CHexActor*>(entity)->chooseTurnAction();
		//TO DO: rather than this mess, have a list of actors
		//not just entities
	}
	turnPhase = playerChoosePhase;

	//refresh cursor path
	cursorPath = calcPath(playerObj->hexPosition, hexCursor->hexPosition);
}

/** Ask all entities to initiate their chosen action for this turn. */
void CHexWorld::startActionPhase() {
	turnPhase = actionPhase;
}

/** Begin a player right-click action. */
void CHexWorld::rightClick() {
	if (turnPhase != playerChoosePhase )
		return;

	if (powerMode)
		return;

	playerObj->setAction(tig::actPlayerMove, hexCursor->hexPosition);
	startActionPhase();
}

/** Begin a player left-click action. */
void CHexWorld::leftClick() {
	if (turnPhase != playerChoosePhase)
		return;

	CGameHexObj* obj = getPrimaryObjectAt(hexCursor->hexPosition);

	if (powerMode)
		return;
	//TO DO: Instead, we'll have a special powerMode leftClick func to call
	//during powermode, which only robots and player need implememnt

	if (obj) {
		obj->leftClick();
		startActionPhase();
		return;
	}


	//empty hex = shoot
	playerObj->setAction(tig::actPlayerShoot, hexCursor->hexPosition );
	startActionPhase();
	return;
}



/** If there are any gridObjects, update the lead one and return true. */
bool CHexWorld::resolvingGridObjActions() {
	for (auto gridObj = gridObjects.begin(); gridObj != gridObjects.end(); gridObj++) {
		if (!(*gridObj)->update(dT)) {
			gridObjects.erase(gridObj);
		}
		return true;
	}
	return false;
}

/** If any entity is performing a serial action, update the lead one and return true. */
bool CHexWorld::resolvingSerialActions() {
	if (serialList.empty())
		return resolved;

	if (currentSerialActor != serialList[0]) {
		currentSerialActor = serialList[0];
		serialList[0]->initAction();
	}

	if (serialList[0]->update(dT) == resolved) {
		serialList.erase(serialList.begin());
		if (serialList.empty()) {
			return resolved;
		}
	}

	return unresolved;
}

/** If any entity is performing a simultaneous action, update each one and return true. */
bool CHexWorld::resolvingSimulActions() {
	bool stillResolving = false;

	if (initSimulActions) {
		for (auto actor : simulList) {
			actor->initAction();
		}
		initSimulActions = false;
	}


	for (auto actor = simulList.begin(); actor != simulList.end();) {
		bool resolving = (*actor)->update(dT);
		if (resolving)
			actor++;
		else
			actor = simulList.erase(actor);
		stillResolving |= resolving;
	}


	if (stillResolving)
		return true;

	turnPhase = chooseActionPhase;
	initSimulActions = true;
	playerObj->onTurnEnd(); liveLog << "\nturn end!";
	return false;
}

/** Fill the map with its entities. */
void CHexWorld::tempPopulateMap() {
	//get pointers to robots
	ITigObj* pRobot = vm->getObject(tig::botA);

	robot = new CRobot();
	robot->setLineModel("robot");
	//robot->setPosition(2, 0, -2);
	//robot->setMap(&map);
	map.add(robot, CHex(2, 0, -2));
	//robot->isRobot = true;
	robot->setTigObj(pRobot);
	

	pRobot = vm->getObject(tig::botB);

	robot2 = new CRobot();
	robot2->setLineModel("robot");
	//robot2->setPosition(4, -1, -3);
	//robot2->setMap(&map);
	map.add(robot2, CHex(4, -1, -3));
	//robot2->isRobot = true;
	robot2->setTigObj(pRobot);

	entities.push_back(robot);
	entities.push_back(robot2);

	wrench = new CHexItem();
	wrench->setLineModel("test");
	//wrench->setPosition(-8, 0, 8);
	//wrench->setMap(&map);
	map.add(wrench, CHex(-7, 0, 7));
	wrench->setTigObj(vm->getObject(tig::monkeyWrench));
	entities.push_back(wrench);

	shield = new CHexItem();
	shield->setLineModel("test");
	//shield->setPosition(-6, -2, 8);
	//shield->setMap(&map);
	map.add(shield, CHex(-5, -2, 7));
	shield->setTigObj(vm->getObject(tig::shield));
	entities.push_back(shield);

	blaster = new CHexItem();
	blaster->setLineModel("test");
	map.add(blaster, CHex(-6, -1, 7));
	blaster->setTigObj(vm->getObject(tig::blaster));
	entities.push_back(blaster);

	door = new CDoor();
	door->setLineModel("door");
	map.add(door, CHex(0, 0, 0));
	door->setTigObj(vm->getObject(tig::CDoor));
	door->setZheight(0.01f); //TO DO: sort this!!!!!
	entities.push_back(door);


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
		popupMsg(strParam); break;
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
	entities.push_back(item);
	map.add(item, location);
}

CGroupItem* CHexWorld::createGroupItem() {
	CGroupItem* groupItem = new CGroupItem();
	groupItem->setLineModel("test");
	groupItem->setTigObj(vm->getObject(tig::CGroupItem));
	entities.push_back(groupItem);
	return groupItem;
}

/** Remove this entity from the entities list. */
void CHexWorld::removeEntity(CGameHexObj& entity) {
	auto removee = std::find(entities.begin(), entities.end(), (CGameHexObj*)&entity );
	if (removee != entities.end())
		entities.erase(removee);
}

/** Remove this grid object from the grid objects list. */
void CHexWorld::removeGridObj(CGridObj& gridObj) {
	auto removee = std::find(gridObjects.begin(), gridObjects.end(), (CGridObj*)&gridObj);
	if (removee != gridObjects.end())
		gridObjects.erase(removee);
}

/** Return the CItem or CGroupItem object found at this hex, if any. */
CGameHexObj* CHexWorld::getItemAt(CHex& position) {
	for (auto entity : entities) {
		if (entity->hexPosition == position && (entity->isTigClass(tig::CItem)
			|| entity->isTigClass(tig::CGroupItem)) )
			return entity;
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

void CHexWorld::removeDeletedEntities() {
	for (auto entity = entities.begin(); entity < entities.end();) {
		if ((*entity)->deleteMe)
			entity = entities.erase(entity);
		else
			entity++;
	}
}

CGridObj* CHexWorld::createBolt() {
	CBolt* bolt = new CBolt();
	bolt->setLineModel(hexRenderer.getLineModel("bolt"));
	gridObjects.push_back(bolt);
	return bolt;
}



/*** Create a popup window with the given text. */
void CHexWorld::popupMsg(const std::string& text) {
	CGameTextWin* textWin = new CGameTextWin();
	textWin->addText(text);
	mainApp->addGameWindow(textWin);
	textWin->resizeToFit();
	textWin->positionAtMousePointer();
	popupWindows.push_back(textWin);
}


/** Add this actor to an action list. */
void CHexWorld::onAddActor(CAddActor& msg) {
	if (msg.addTo == actionSimul)
		simulList.push_back(msg.actor);
	else
		serialList.push_back(msg.actor);
}

/** Fire a bolt at the target. */
void CHexWorld::onShootAt(CShootAt& msg) {
	CBolt* boltTmp = (CBolt*)createBolt();
	boltTmp->setPosition(msg.start);
	boltTmp->fireAt(msg.target);
}

void CHexWorld::onDropItem(CDropItem& msg) {
	dropItem(msg.item, msg.location);
}

void CHexWorld::onRemoveEntity(CRemoveEntity& msg) {
	removeEntity(*msg.entity);
}

void CHexWorld::onCreateGroupItem(CCreateGroupItem& msg) {
	CGroupItem* groupItem = createGroupItem();
	groupItem->items.push_back(msg.item1);
	groupItem->items.push_back(msg.item2);
	removeEntity(*msg.item2);
	groupItem->setPosition(playerObj->hexPosition);
}

/** Return the highest priority object at this hex. ie, the one if 
	clicked-on we would want to respond. */
CGameHexObj* CHexWorld::getPrimaryObjectAt(CHex& hex) {
	TRange entities = map.getEntitiesAt(hex);

	for (auto it = entities.first; it != entities.second; it++) {
		if (it->second->isTigClass(tig::CRobot2))
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

	return NULL;
}



