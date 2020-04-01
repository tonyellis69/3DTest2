#include "hexWorld.h"

#include <string>

#include "utils/log.h"

#include "gameTextWin.h"

CHexWorld::CHexWorld() {
	hexRenderer.setCallbackApp(this);
	turnPhase = chooseActionPhase;
	CHexObject::setHexRenderer(&hexRenderer);
	CGridObj::setHexRenderer(&hexRenderer);
	CGridObj::setHexWorld(this);
	CGameHexObj::setHexWorld(this);
}

/** Provide a pointer to the game app to check for mouse input, etc. */
void CHexWorld::setMainApp(IMainApp* pApp) {
	this->mainApp = pApp;	
}

void CHexWorld::setVM(Ivm* pVM) {
	vm = pVM;
	mapMaker.attachVM(pVM);
	setTigObj(vm->getObject(tig::IHexWorld)); 
}

/**	Load a multipart mesh for storage under the given name. */
void CHexWorld::addMesh(const std::string& name, CMesh& mesh) {
	CBuf* meshBuf = hexRenderer.createMeshBuffer(name);
	mesh.exportToBuffer(*meshBuf);
}

void CHexWorld::makeMap(ITigObj* tigMap) {
	mapMaker.attachMapObject(tigMap);
	map = mapMaker.createMap();
}

/** Late set-up stuff. */
void CHexWorld::start() {
	temporaryCreateHexObjects();

	

	tempPopulateMap();

	hexRenderer.setMap(&map);

	map.setEntityList(&entities);

	hexRenderer.start();	



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
}

void CHexWorld::setPlayerShield(THexDir direction) {
	playerObj->setShield(direction);
}

void CHexWorld::onMouseWheel(float delta) {
	if (mainApp->hexKeyNowCallback(GLFW_KEY_LEFT_SHIFT)) {
		hexRenderer.pitchCamera(delta);
	}
	else
		hexRenderer.dollyCamera(delta);
}

void CHexWorld::onMouseMove(int x, int y, int key) {
	CHex mouseHex = hexRenderer.pickHex(x, y);
	if (mouseHex != hexCursor->hexPosition)
		onNewMouseHex(mouseHex);
}



void CHexWorld::draw() {
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

	if (turnPhase == actionPhase) { 	
		if (resolvingGridObjActions())
			return;

		if (resolvingSerialActions())
			return;

		if (resolvingSimulActions()) {
			return;
		}
	}

	else if (turnPhase == chooseActionPhase) {
		chooseActions();
	}
}


/////////////public - private devide

/** Plot a navigable path from start to end on the current map. */
THexList CHexWorld::calcPath(CHex& start, CHex& end) {
	return map.aStarPath(start,end);
}

/** Return the hex entity at the given hex, if any. */
CGameHexObj* CHexWorld::getEntityAt(CHex& hex) {
	for (auto entity : entities) {
		if (entity->hexPosition == hex)
			return entity;
	}
	return NULL;
}

/** Returns true if a path-blocking entity is currently moving to the given hex. */
bool CHexWorld::isBlockerMovingTo(CHex& hex) {
	for (auto entity : entities) {
		if (entity->destination == hex && entity->blocks)
			return true;
	}
	return false;
}

/** Because lots of code want to know. */
CHex CHexWorld::getPlayerPosition() {
	return playerObj->hexPosition;
}


CHex CHexWorld::getPlayerDestination() {
	return playerObj->destination;
}


/** TO DO: temporary.This should not be hard-coded, but spun from a level-maker. */
void CHexWorld::temporaryCreateHexObjects() {
	playerObj = new CPlayerObject();
	playerObj->setBuffer(hexRenderer.getBuffer("player"));
	//playerObj->setPosition(5, -3, -2);
	playerObj->setPosition(-1, 9, -8);
	playerObj->shieldBuf = hexRenderer.getBuffer("shield");
	playerObj->setTigObj(vm->getObject("player"));
	playerObj->setMap(&map);

	entities.push_back(playerObj);

	hexCursor = new CGameHexObj();
	hexCursor->setBuffer(hexRenderer.getBuffer("cursor"));
	hexCursor->setPosition(0, 0, 0);
}


/** Respend to mouse cursor moving to a new hex. */
void CHexWorld::onNewMouseHex(CHex& mouseHex) {
	hexCursor->setPosition(mouseHex);
	playerObj->calcTravelPath(hexCursor->hexPosition);	

	for (auto win : popupWindows)
		delete win;
	popupWindows.clear();
	
	CGameHexObj* entity = getEntityAt(mouseHex);
	if (entity)
		entity->onMouseOver();
}


/** Return the player object's current travel path. */
THexList* CHexWorld::getPlayerPath() {
	return &playerObj->getTravelPath();
}

CGameHexObj* CHexWorld::getPlayerObj() {
	return playerObj;
}

CHexObject* CHexWorld::getCursorObj() {
	return hexCursor;
}

/** All entities choose their action for the coming turn. */
void CHexWorld::chooseActions() {
	serialActions.clear();
	for (auto entity : entities) {
		entity->chooseTurnAction();
		if (entity->isResolvingSerialAction())
			serialActions.push_back(entity);
		//TO DO: have another list for simultaneous actions
		//rather than checl all entities
	}
	turnPhase = playerChoosePhase;
}

/** Ask all entities to initiate their chosen action for this turn. */
void CHexWorld::startActionPhase() {
	turnPhase = actionPhase;
	for (auto entity : entities) { //TO DO: make this redundant
		entity->beginTurnAction();
	}
}

/** Begin a player right-click action. */
void CHexWorld::beginRightClickAction() {
	if (turnPhase != playerChoosePhase )
		return;

	CGameHexObj* clickedEntity = getEntityAt(hexCursor->hexPosition);
	if (!clickedEntity || !playerObj->isNeighbour(*clickedEntity))
		playerObj->stackAction({ tig::actPlayerMove,NULL });

	startActionPhase();
}

/** Begin a player left-click action. */
void CHexWorld::beginLeftClickAction() {
	if (turnPhase != playerChoosePhase)
		return;

	CGameHexObj* clickedEntity = getEntityAt(hexCursor->hexPosition);
	if (clickedEntity) {
		if (playerObj->isNeighbour(*clickedEntity))
			if (clickedEntity->isTigClass(tig::CItem))
				playerObj->takeItem(*clickedEntity);
			else
				addSerialAction(playerObj, { tig::actPlayerMeleeAttack, clickedEntity });
		else {
			addSerialAction(playerObj, { tig::actPlayerShoot, hexCursor });
		}
	}
	else
		addSerialAction(playerObj, { tig::actPlayerShoot, hexCursor });
		
		
	startActionPhase();
}


/** Start the player down its travel path, if any. */
bool CHexWorld::beginPlayerMove() {
	 return playerObj->beginMove();
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
	for (auto entity = serialActions.begin(); entity != serialActions.end(); entity++) {
		if (!(*entity)->update(dT)) {
			serialActions.erase(entity);
		}
		return true;
	}
	return false;
}

/** If any entity is performing a simultaneous action, update each one and return true. */
bool CHexWorld::resolvingSimulActions() {
	bool resolvingActions = false;
	for (auto entity : entities) {
		resolvingActions |= entity->update(dT);
	}

	if (resolvingActions)
		return true;

	turnPhase = chooseActionPhase;
	return false;
}

/** Fill the map with its entities. */
void CHexWorld::tempPopulateMap() {
	//get pointers to robots
	ITigObj* pRobot = vm->getObject(tig::botA);

	robot = new CRobot();
	robot->setBuffer(hexRenderer.getBuffer("robot"));
	robot->setPosition(2, 0, -2);
	robot->isRobot = true;
	robot->setTigObj(pRobot);
	robot->setMap(&map);

	pRobot = vm->getObject(tig::botB);

	robot2 = new CRobot();
	robot2->setBuffer(hexRenderer.getBuffer("robot"));
	robot2->setPosition(4, -1, -3);
	robot2->isRobot = true;
	robot2->setTigObj(pRobot);
	robot2->setMap(&map);

	entities.push_back(robot);
	entities.push_back(robot2);

	wrench = new CHexItem();
	wrench->setBuffer(hexRenderer.getBuffer("test"));
	wrench->setPosition(-8, 0, 8);
	wrench->setTigObj(vm->getObject(tig::monkeyWrench));
	entities.push_back(wrench);

	shield = new CHexItem();
	shield->setBuffer(hexRenderer.getBuffer("test"));
	shield->setPosition(-6, -2, 8);
	shield->setTigObj(vm->getObject(tig::shield));
	entities.push_back(shield);

	blaster = new CHexItem();
	blaster->setBuffer(hexRenderer.getBuffer("test"));
	blaster->setPosition(-7, -1, 8);
	blaster->setTigObj(vm->getObject(tig::blaster));
	entities.push_back(blaster);


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
}

CGroupItem* CHexWorld::createGroupItem() {
	CGroupItem* groupItem = new CGroupItem();
	groupItem->setBuffer(hexRenderer.getBuffer("test"));
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
	bolt->setBuffer(hexRenderer.getBuffer("bolt"));
	gridObjects.push_back(bolt);
	return bolt;
}

void CHexWorld::addToSerialActions(CGameHexObj* entity) {
	serialActions.insert(serialActions.begin(), entity);
}

/** Assign this entity the given action, and add it to
	the list of entities with a serial action to perform. */
void CHexWorld::addSerialAction(CGameHexObj* entity, CAction action) {
	entity->stackAction(action);
	serialActions.insert(serialActions.begin(), entity);
	//TO DO: might be useful to ensure entities can only be added
	//once, maybe use a set.
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




