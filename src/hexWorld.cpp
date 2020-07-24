#include "hexWorld.h"

#include <string>

#include "utils/log.h"

#include "gameTextWin.h"

CGameHexObj nullGameHexObj;

CHexWorld::CHexWorld() {
	CHexObject::setHexRenderer(&hexRenderer);
	CGridObj::setHexRenderer(&hexRenderer);
	
	mapMaker.entities = &entities;

	messageBus.setHandler<CAddActor>(this, &CHexWorld::onAddActor);
	messageBus.setHandler<CShootAt>(this, &CHexWorld::onShootAt);
	messageBus.setHandler<CDropItem>(this, &CHexWorld::onDropItem);
	messageBus.setHandler<CRemoveEntity>(this, &CHexWorld::onRemoveEntity);
	messageBus.setHandler<CCreateGroupItem>(this, &CHexWorld::onCreateGroupItem);
	messageBus.setHandler<CMissileHit>(this, &CHexWorld::onMissileHit);
	messageBus.setHandler<CKill>(this, &CHexWorld::onKill);
	messageBus.setHandler<CDiceRoll>(this, &CHexWorld::onDiceRoll);
	messageBus.setHandler<CPlayerNewHex>(this, &CHexWorld::onPlayerNewHex);
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
	createCursorObject();
	tempPopulateMap();

	hexRenderer.setMap(&map);
	map.setEntityList(&entities);
	hexRenderer.start();	

	hexPosLbl = new CGUIlabel2(10, 10, 510, 30);
	hexPosLbl->setTextColour(glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f ));
	mainApp->addGameWindow(hexPosLbl);

	startGame();
}

/** Required each time we restart. */
void CHexWorld::startGame() {
	

	//remove any existing actors
	killEntity(robot);
	killEntity(robot2);
	killEntity(playerObj);

	//create new player object
	playerObj = new CPlayerObject();
	playerObj->setLineModel("player");
	map.add(playerObj, CHex(-13, 5, 8));
	playerObj->setTigObj(vm->getObject("player"));
	entities.push_back(playerObj);
	subscribe(playerObj);

	//create two new robots
	ITigObj* pRobot = vm->getObject(tig::botA);

	robot = new CRobot();
	robot->setLineModel("robot");
	map.add(robot, CHex(2, 0, -2));
	robot->setTigObj(pRobot);


	pRobot = vm->getObject(tig::botB);

	/*robot2 = new CRobot();
	robot2->setLineModel("robot");
	map.add(robot2, CHex(4, -1, -3));
	robot2->setTigObj(pRobot);*/

	entities.push_back(robot);
	//entities.push_back(robot2);

	subscribe(robot);
	//subscribe(robot2);
	robot->setGoalWander();

	turnPhase = actionPhase;
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
	if (turnPhase == playerDeadPhase) {
		//TO DO: temp! player death should turn off input at the source
		return;
	}
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
	if (turnPhase == playerChoosePhase) {
		hexRenderer.draw();
		hexCursor->draw();
		if (!powerMode)
			hexRenderer.drawPath(&cursorPath, glm::vec4{ 0.6, 0.4, 1, 0.1f }, glm::vec4{ 0.6, 0.4, 1, 0.75f });
	}
	else {
		hexRenderer.draw();
		//hexCursor->draw();
	}

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

	updateCameraPosition();

	for (auto entity : entities)
		entity->frameUpdate(dT);
	
	//resolvingGridObjActions();

	if (turnPhase == actionPhase) { 
		if (resolvingGridObjActions())
			return;

		if (resolvingSerialActions())
			return;

		if (resolvingSimulActions()) {
			return;
		}

		//end of action phase
		endTurn();

		beginNewTurn();
	}
}


/** User has triggered the ctrl left mouse  event. */
void CHexWorld::onCtrlLMouse() {
	
}

/** User has released set-defence key. */
void CHexWorld::powerKeyRelease() {
	//for now, just do this
	powerMode = false;
	CSysMsg msg(powerMode);
	send(msg);
}

/** Player has pressed the power mode on/off key. */
void CHexWorld::powerKeyDown() {
	//TO DO: call func on QPS
	//for now, just do this
	powerMode = true;
	CSysMsg msg(powerMode);
	send(msg);
}

/** Player has pressed the enter key. */
void CHexWorld::enterKeyDown() {
	if (turnPhase == playerDeadPhase)
		startGame();
}


/////////////public - private devide

/** Plot a navigable path from start to end on the current map. */
THexList CHexWorld::calcPath(CHex& start, CHex& end) {
	return map.aStarPath(start,end);
}


void CHexWorld::createCursorObject() {
	hexCursor = new CGameHexObj();
	hexCursor->setLineModel("cursor");
	hexCursor->setPosition(CHex(0, 0, 0));
}


/** Respond to mouse cursor moving to a new hex. */
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

	if (hexCursor->hexPosition == playerObj->hexPosition ||
		map.fromToBlocked(playerObj->hexPosition, hexCursor->hexPosition) )
		return;
	playerObj->setAction(tig::actPlayerMove, hexCursor->hexPosition);
	startActionPhase();
}

/** Begin a player left-click action. */
void CHexWorld::leftClick() {
	if (turnPhase != playerChoosePhase)
		return;

	CGameHexObj* obj = getPrimaryObjectAt(hexCursor->hexPosition);

	if (powerMode) {
		obj->leftClickPowerMode();
		return;
	}

	if (obj != &nullGameHexObj) {
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

/** Fill the map with its permanent entities. */
void CHexWorld::tempPopulateMap() {
	wrench = new CHexItem();
	wrench->setLineModel("test");
	map.add(wrench, CHex(-7, 0, 7));
	wrench->setTigObj(vm->getObject(tig::monkeyWrench));
	entities.push_back(wrench);

	shield = new CHexItem();
	shield->setLineModel("test");
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
	map.add(door, CHex(13, -4, -9));
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
void CHexWorld::removeFromEntityList(CGameHexObj& entity) {
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

/** Called when someone wants to shoot a missile at a target or hex.*/
void CHexWorld::onShootAt(CShootAt& msg) {
	CBolt* boltTmp = (CBolt*)createBolt();
	boltTmp->setPosition(msg.start);
	boltTmp->damage = msg.damage;
	boltTmp->fireAt(msg.attacker, msg.target);
}

void CHexWorld::onDropItem(CDropItem& msg) {
	dropItem(msg.item, msg.location);
}

void CHexWorld::onRemoveEntity(CRemoveEntity& msg) {
	removeFromEntityList(*msg.entity);
}

void CHexWorld::onCreateGroupItem(CCreateGroupItem& msg) {
	CGroupItem* groupItem = createGroupItem();
	groupItem->items.push_back(msg.item1);
	groupItem->items.push_back(msg.item2);
	removeFromEntityList(*msg.item2);
	groupItem->setPosition(playerObj->hexPosition);
}

/** Handle a missile landing in this hex. */
void CHexWorld::onMissileHit(CMissileHit& msg) {
	//Anything here?
	//if so, damage it
	CGameHexObj* obj = getPrimaryObjectAt(msg.hex);
	if (obj != &nullGameHexObj)
		obj->receiveDamage(*msg.attacker, msg.damage);
}

/** Handle an entity's request to be killed off. */
void CHexWorld::onKill(CKill& msg) {
	killEntity(msg.entity);
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
}

/** Return the highest priority object at this hex. ie, the one if 
	clicked-on we would want to respond. */
CGameHexObj* CHexWorld::getPrimaryObjectAt(CHex& hex) {
	TRange entities = map.getEntitiesAt(hex);

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
	if (hexRenderer.following())
		hexRenderer.followTarget(playerObj->worldPos);
	else
		hexRenderer.attemptScreenScroll(mainApp->getMousePos(), dT);
}

void CHexWorld::beginNewTurn() {
	qps.beginNewTurn();
	playerObj->onTurnBegin();
	map.updateBlocking();
	chooseActions();

	///////////////////////////temp end stuff
	if (playerObj->hexPosition == door->hexPosition) {
		CSendText msg(combatLog, "\n\nYOU HAVE WON");
		send(msg);

		killEntity(robot);
		killEntity(robot2);
		turnPhase = playerDeadPhase;
	}
}

void CHexWorld::endTurn() {
	qps.endTurn();
}

void CHexWorld::killEntity(CGameHexObj* entity) {
	auto simul = std::find(simulList.begin(), simulList.end(), entity);
	if (simul != simulList.end())
		simulList.erase(simul);

	auto serial = std::find(serialList.begin(), serialList.end(), entity);
	if (serial != serialList.end())
		serialList.erase(serial);

	map.removeFromEntityList(entity);

	removeFromEntityList(*entity);

	if (entity == playerObj)
		turnPhase = playerDeadPhase;
}


