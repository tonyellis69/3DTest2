#include "hexWorld.h"

#include "utils/log.h"

CHexWorld::CHexWorld() {
	hexRenderer.setCallbackApp(this);
	turnPhase = chooseActionPhase;
	CHexObject::setHexRenderer(&hexRenderer);
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

	if (pCallbackApp->hexMouseButtonNowCallback(GLFW_MOUSE_BUTTON_LEFT)) {
		if ( turnPhase == playerChoosePhase &&  !playerObj.moving) {
			beginLeftClickAction();
		}
	}

	
}

/** Called when a key is pressed. */
void CHexWorld::onKeyDown(int key, long mod) {
	if (key == GLFW_KEY_KP_6) {
		playerObj.setShield(hexEast);
	}
	else if (key == GLFW_KEY_KP_3) {
		playerObj.setShield(hexSE);
	}
	else if (key == GLFW_KEY_KP_1) {
		playerObj.setShield(hexSW);
	}
	else if (key == GLFW_KEY_KP_4) {
		playerObj.setShield(hexWest);
	}
	else if (key == GLFW_KEY_KP_7) {
		playerObj.setShield(hexNW);
	}
	else if (key == GLFW_KEY_KP_9) {
		playerObj.setShield(hexNE);
	}

	if (key == 'I')
		playerObj.showInventory();

	if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
		int item = key - GLFW_KEY_1;
		playerObj.dropItem(item);
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
	if (mouseHex != hexCursor.hexPosition)
		onCursorMove(mouseHex);
}


void CHexWorld::onMouseButton(int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT &&  action == GLFW_PRESS) {
		if  (turnPhase == playerChoosePhase)  
			beginLeftClickAction();	
	}
}


void CHexWorld::draw() {
	hexRenderer.draw();
	for (auto entity : entities)
		entity->draw();
}

/** Adjust horizontal vs vertical detail of the view. Usually called when the screen size changes. */
void CHexWorld::setAspectRatio(glm::vec2& ratio) {
	hexRenderer.setCameraAspectRatio(ratio);
}


/** Called every frame to get the hex world up to date.*/
void CHexWorld::update(float dT) {
	this->dT = dT;

	if (turnPhase == actionPhase) { 		
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
	return playerObj.hexPosition;
}

CHex CHexWorld::getPlayerDestinationCB() {
	return playerObj.destination;
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
	playerObj.setBuffer(hexRenderer.getBuffer("player"));
	//playerObj.setPosition(5, -3, -2);
	playerObj.setPosition(-9, 1, 8);
	playerObj.shieldBuf = hexRenderer.getBuffer("shield");
	playerObj.setTigObj(vm->getObject("player"));


	entities.push_back(&playerObj);


	hexCursor.setBuffer(hexRenderer.getBuffer("cursor"));
	hexCursor.setPosition(0, 0, 0);

}



/** Respend to cursor moving to a new hex. */
void CHexWorld::onCursorMove(CHex& mouseHex) {
	hexCursor.setPosition(mouseHex);
	playerObj.calcTravelPath(hexCursor.hexPosition);	
}



/** Return the player object's current travel path. */
THexList* CHexWorld::getPlayerPath() {
	return &playerObj.getTravelPath();
}

CHexObject* CHexWorld::getCursorObj(){
	return &hexCursor;
}

CGameHexObj* CHexWorld::getPlayerObj() {
	return &playerObj;
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
void CHexWorld::beginLeftClickAction() {
	CGameHexObj* entity = getEntityAt(hexCursor.hexPosition);
	if (entity )
		entity->onLeftClick();
	
	if (!entity || !playerObj.isNeighbour(*entity))
		beginPlayerMove();
	
	startActionPhase();
}


/** Initiate a player lunge attack on the target. */
void CHexWorld::beginPlayerLunge(CGameHexObj& target) {
	playerObj.beginAttack(target);
	serialActions.insert(serialActions.begin(), &playerObj);
}

/** Start the player down its travel path, if any. */
bool CHexWorld::beginPlayerMove() {
	 return playerObj.beginMove();
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

	robot.setBuffer(hexRenderer.getBuffer("robot"));
	robot.setPosition(2, 0, -2);
	robot.isRobot = true;
	robot.setTigObj(pRobot);

	pRobot = vm->getObject(tig::botB);

	robot2.setBuffer(hexRenderer.getBuffer("robot"));
	robot2.setPosition(4, -1, -3);
	robot2.isRobot = true;
	robot2.setTigObj(pRobot);

	entities.push_back(&robot);
	entities.push_back(&robot2);

	wrench.setBuffer(hexRenderer.getBuffer("test"));
	wrench.setPosition(-8, 0, 8);
	wrench.setTigObj(vm->getObject(tig::monkeyWrench));
	entities.push_back(&wrench);

	shield.setBuffer(hexRenderer.getBuffer("test"));
	shield.setPosition(-6, -2, 8);
	shield.setTigObj(vm->getObject(tig::shield));
	entities.push_back(&shield);

	blaster.setBuffer(hexRenderer.getBuffer("test"));
	blaster.setPosition(-7, -1, 8);
	blaster.setTigObj(vm->getObject(tig::blaster));
	entities.push_back(&blaster);


}

int CHexWorld::tigCall(int memberId) {
	std::string msg = vm->getParamStr(0);
	liveLog << msg;
	return 1;
}

/** Cope with the player taking an item. */
void CHexWorld::playerTake(CGameHexObj& item) {
	auto it = std::find(entities.begin(), entities.end(), &item);
	if (it != entities.end()) {
		playerItems.push_back(&item);
		entities.erase(it);
	}
}

void CHexWorld::playerDrop(CGameHexObj* item) {
	auto it = std::find(playerItems.begin(), playerItems.end(), item);
	if (it != playerItems.end()) {
		(*it)->setPosition(playerObj.hexPosition);
		entities.push_back(item);
		playerItems.erase(it);
	}
}


