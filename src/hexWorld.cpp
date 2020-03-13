#include "hexWorld.h"

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
void CHexWorld::setCallbackApp(IhexWorldCallback* pApp) {
	pCallbackApp = pApp;	
}

void CHexWorld::setVM(Ivm* pVM) {
	vm = pVM;
	mapMaker.attachVM(pVM);
	setTigObj(vm->getObject(tig::CConsole)); ///TO DO: temp!
}

/**	Load a single mesh for storage under the given name. */
void CHexWorld::addMesh(const std::string& name, std::vector<CMesh>& meshes) {
	CBuf* meshBuf = hexRenderer.addBuffer(name);
	meshes[0].writeToBufferv3i(*meshBuf);
}

/**	Load a multipart mesh for storage under the given name. */
void CHexWorld::addMesh(const std::string& name, CMesh& mesh) {
	CBuf* meshBuf = hexRenderer.addBuffer(name);
	mesh.writeToBufferv3i(*meshBuf);
}

/** Late set-up stuff. */
void CHexWorld::start() {
	createHexObjects();

	hexArray = mapMaker.createMap();

	populateMap();

	hexRenderer.setMap(&hexArray);

	hexArray.setEntityList(&entities);

	hexRenderer.start();	

	//temp!!!!
	CGameTextWin* textWin = new CGameTextWin();
	textWin->addText("Here is some test text.");
//	pCallbackApp->addGameWindow(textWin);
}

/** Check and respond to keys being pressed this frame. */
void CHexWorld::keyCheck() {
	float camSpeed = 6.0f * dT;

	if (pCallbackApp->hexKeyNowCallback('W')) {
		hexRenderer.moveCamera(glm::vec3{ 0, camSpeed, 0 });
	} 
	else if (pCallbackApp->hexKeyNowCallback('S')) {
		hexRenderer.moveCamera(glm::vec3{ 0, -camSpeed, 0 });
	} 
	else if (pCallbackApp->hexKeyNowCallback('A')) {
		hexRenderer.moveCamera(glm::vec3{ -camSpeed,0,0 });
	} 
	else if (pCallbackApp->hexKeyNowCallback('D')) {
		hexRenderer.moveCamera(glm::vec3{ camSpeed,0,0 });
	}

	//happens if button held down
	if (pCallbackApp->hexMouseButtonNowCallback(GLFW_MOUSE_BUTTON_RIGHT)) {
		if ( turnPhase == playerChoosePhase &&  !playerObj->moving) {
			beginRightClickAction();
		}
	}

	
}

/** Called when a key is pressed. */
void CHexWorld::onKeyDown(int key, long mod) {
	if (key == GLFW_KEY_KP_6) {
		playerObj->setShield(hexEast);
	}
	else if (key == GLFW_KEY_KP_3) {
		playerObj->setShield(hexSE);
	}
	else if (key == GLFW_KEY_KP_1) {
		playerObj->setShield(hexSW);
	}
	else if (key == GLFW_KEY_KP_4) {
		playerObj->setShield(hexWest);
	}
	else if (key == GLFW_KEY_KP_7) {
		playerObj->setShield(hexNW);
	}
	else if (key == GLFW_KEY_KP_9) {
		playerObj->setShield(hexNE);
	}

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

void CHexWorld::onMouseWheel(float delta) {
	if (pCallbackApp->hexKeyNowCallback(GLFW_KEY_LEFT_SHIFT)) {
		hexRenderer.pitchCamera(delta);
	}
	else
		hexRenderer.dollyCamera(delta);
}

void CHexWorld::onMouseMove(int x, int y, int key) {
	CHex mouseHex = hexRenderer.pickHex(x, y);
	if (mouseHex != hexCursor->hexPosition)
		onCursorMove(mouseHex);
}


void CHexWorld::onMouseButton(int button, int action, int mods) {
	if (turnPhase != playerChoosePhase)
		return;
	//happens when button initially pressed down
	if (button == GLFW_MOUSE_BUTTON_RIGHT &&  action == GLFW_PRESS) {
		beginRightClickAction();	
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		beginLeftClickAction();
	}
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

void CHexWorld::setMap(ITigObj* map) {
	mapMaker.attachMapObject(map);
}




/** Provides a callback function for using hexArray's pathfinding facility. */
THexList CHexWorld::calcPath(CHex& start, CHex& end) {
	return hexArray.aStarPath(start,end);
}

/** A callback function for finding what entity is at the given hex. */
CGameHexObj* CHexWorld::getEntityAt(CHex& hex) {
	for (auto entity : entities) {
		if (entity->hexPosition == hex)
			return entity;
	}
	return NULL;
}

bool CHexWorld::isBlockerMovingTo(CHex& hex) {
	for (auto entity : entities) {
		if (entity->destination == hex && entity->blocks)
			return true;
	}
	return false;
}

/** Called to initiate the rest of the world's turn to act. */
void CHexWorld::onPlayerTurnDoneCB() {
	
}

CHex CHexWorld::getPlayerPosition() {
	return playerObj->hexPosition;
}

CHex CHexWorld::getPlayerDestinationCB() {
	return playerObj->destination;
}

bool CHexWorld::isEntityDestinationCB(CHex& hex) {
	for (auto entity : entities) {
		if (hex == entity->destination)
			return true;
	}
	return false;
}

///////////////////////Private functions/////////////////////////////////

/** TO DO: temporary.This should not be hard-coded, but spun from a level-maker. */
void CHexWorld::createHexObjects() {
	playerObj = new CPlayerObject();
	playerObj->setBuffer(hexRenderer.getBuffer("player"));
	//playerObj->setPosition(5, -3, -2);
	playerObj->setPosition(-9, 1, 8);
	playerObj->shieldBuf = hexRenderer.getBuffer("shield");
	playerObj->setTigObj(vm->getObject("player"));


	entities.push_back(playerObj);

	hexCursor = new CHexObject();
	hexCursor->setBuffer(hexRenderer.getBuffer("cursor"));
	hexCursor->setPosition(0, 0, 0);




}



/** Respend to cursor moving to a new hex. */
void CHexWorld::onCursorMove(CHex& mouseHex) {
	hexCursor->setPosition(mouseHex);
	playerObj->calcTravelPath(hexCursor->hexPosition);	
}



/** Return the player object's current travel path. */
THexList* CHexWorld::getPlayerPath() {
	return &playerObj->getTravelPath();
}

CHexObject* CHexWorld::getCursorObj(){
	return hexCursor;
}

CGameHexObj* CHexWorld::getPlayerObj() {
	return playerObj;
}

int CHexWorld::diceRoll(int dice) {
	std::uniform_int_distribution roll{ 1,dice };
	return roll(randEngine);
}



/** All entities choose their action for the coming turn. */
void CHexWorld::chooseActions() {
	serialActions.clear();
	for (auto entity : entities) {
		entity->chooseTurnAction();
		if (entity->isResolvingSerialAction())
			serialActions.push_back(entity);
	}
	turnPhase = playerChoosePhase;
}

/** Ask all entities to initiate their chosen action for this turn. */
void CHexWorld::startActionPhase() {
	turnPhase = actionPhase;
	for (auto entity : entities) {
		entity->beginTurnAction();
	}
}

/** Begin a player left-click action. */
void CHexWorld::beginRightClickAction() {
	CGameHexObj* entity = getEntityAt(hexCursor->hexPosition);
	if (entity )
		entity->onLeftClick();
	
	if (!entity || !playerObj->isNeighbour(*entity))
		beginPlayerMove();
	
	startActionPhase();
}

/** Begin a player right-click action. */
void CHexWorld::beginLeftClickAction() {
	beginPlayerShot();
	startActionPhase();
}


/** Initiate a player lunge attack on the target. */
void CHexWorld::beginPlayerLunge(CGameHexObj& target) {
	playerObj->beginAttack(target);
	serialActions.insert(serialActions.begin(), playerObj);
}

/** Initiate a player shot action .*/
void CHexWorld::beginPlayerShot() {
	CHex endHex = hexArray.findLineEnd(playerObj->hexPosition, hexCursor->hexPosition);
	CBolt* boltTmp = (CBolt*)createBolt();
	boltTmp->setPosition(playerObj->hexPosition);
	boltTmp->fireAt(endHex);
}

/** Start the player down its travel path, if any. */
bool CHexWorld::beginPlayerMove() {
	 return playerObj->beginMove();
}

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
void CHexWorld::populateMap() {
	//get pointers to robots
	ITigObj* pRobot = vm->getObject(tig::botA);

	robot = new CRobot();
	robot->setBuffer(hexRenderer.getBuffer("robot"));
	robot->setPosition(2, 0, -2);
	robot->isRobot = true;
	robot->setTigObj(pRobot);

	pRobot = vm->getObject(tig::botB);

	robot2 = new CRobot();
	robot2->setBuffer(hexRenderer.getBuffer("robot"));
	robot2->setPosition(4, -1, -3);
	robot2->isRobot = true;
	robot2->setTigObj(pRobot);

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
	std::string msg = vm->getParamStr(0);
	liveLog << msg;
	return 1;
}

/** Cope with the player taking an item. */
void CHexWorld::playerTake(CGameHexObj& item) {
	removeEntity(item);
	playerItems.push_back(&item);
}

/** Cope with the player dropping an item. */
void CHexWorld::playerDrop(CGameHexObj* item) {
	auto playerItem = std::find(playerItems.begin(), playerItems.end(), item);

	//is there already an item where we're dropping?
	CGameHexObj* existingItem = getItemAt(playerObj->hexPosition);
	if (existingItem) {
		existingItem->droppedOnBy(*item);
		playerItems.erase(playerItem);
		return;
	}

	(*playerItem)->setPosition(playerObj->hexPosition);
	entities.push_back(item);
	playerItems.erase(playerItem);

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

CHex CHexWorld::findLineEnd(CHex& start, CHex& target) {
	return hexArray.findLineEnd(start, target);

}
