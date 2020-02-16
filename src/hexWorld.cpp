#include "hexWorld.h"

#include "utils/log.h"

CHexWorld::CHexWorld() {
	hexRenderer.setCallbackApp(this);
	turnPhase = chooseActionPhase;
	CHexObject::setHexRenderer(&hexRenderer);
	
}

/** Provide a pointer to the game app to check for mouse input, etc. */
void CHexWorld::setCallbackApp(IhexWorldCallback* pApp) {
	pCallbackApp = pApp;
	
}

void CHexWorld::setVM(Ivm* pVM) {
	vm = pVM;
	mapMaker.attachVM(pVM);
	setTigObj(vm->getObject("CConsole")); ///TO DO: temp!
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
		if  (turnPhase == playerChoosePhase)   //(!resolvingActions)
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
CHexObject* CHexWorld::getEntityAt(CHex& hex) {
	for (auto entity : entities) {
		if (entity->hexPosition == hex)
			return entity;
	}
	return NULL;
}

CHexObject* CHexWorld::entityMovingTo(CHex& hex) {
	for (auto entity : entities) {
		if (entity->destination == hex)
			return entity;
	}
	return NULL;
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
	//playerObj.setPosition(0, 0, 0);
	playerObj.setPosition(5, -3, -2);
	playerObj.setHexWorld(this);
	playerObj.shieldBuf = hexRenderer.getBuffer("shield");
	playerObj.setTigObj(vm->getObject("player"));

	/*robot.setBuffer(hexRenderer.getBuffer("robot"));
	robot.setPosition(2, 0,-2);
	robot.isRobot = true;
	robot.setHexWorld(this);

	robot2.setBuffer(hexRenderer.getBuffer("robot"));
	//robot2.setPosition(5, -5);
	robot2.setPosition(4, -1,-3);
	robot2.isRobot = true;
	robot2.setHexWorld(this);*/

	entities.push_back(&playerObj);
	//entities.push_back(&robot);
	//entities.push_back(&robot2);

	hexCursor.setBuffer(hexRenderer.getBuffer("cursor"));
	hexCursor.setPosition(0, 0, 0);

}



/** Respend to cursor moving to a new hex. */
void CHexWorld::onCursorMove(CHex& mouseHex) {
	hexCursor.setPosition(mouseHex);
	playerObj.calcTravelPath(hexCursor.hexPosition);	
}

/** Return a pointer to the list of entities for the current map. */
TEntities* CHexWorld::getEntities() {
	return &entities;
}

/** Return the player object's current travel path. */
THexList* CHexWorld::getPlayerPath() {
	return &playerObj.getTravelPath();
}

CHexObject* CHexWorld::getCursorObj(){
	return &hexCursor;
}

CHexObject* CHexWorld::getPlayerObj() {
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
	CHexObject* entity = getEntityAt(hexCursor.hexPosition);
	if (entity && entity->isNeighbour(playerObj.hexPosition) )
		beginPlayerLunge(*entity);
	else 
		beginPlayerMove();
	
	startActionPhase();
}


/** Initiate a player lunge attack on the target. */
void CHexWorld::beginPlayerLunge(CHexObject& target) {
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
	ITigObj* pRobot = vm->getObject("botA");

	robot.setBuffer(hexRenderer.getBuffer("robot"));
	robot.setPosition(2, 0, -2);
	robot.isRobot = true;
	robot.setHexWorld(this);
	robot.setTigObj(pRobot);

	pRobot = vm->getObject("botB");

	robot2.setBuffer(hexRenderer.getBuffer("robot"));
	robot2.setPosition(4, -1, -3);
	robot2.isRobot = true;
	robot2.setHexWorld(this);
	robot2.setTigObj(pRobot);

	entities.push_back(&robot);
	entities.push_back(&robot2);

}

void CHexWorld::tigCall(int memberId) {
	//get stack values from some friendly vm interface func
	std::string msg = vm->getStackValueStr();
	
}
