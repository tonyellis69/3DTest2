#include "hexWorld.h"

#include "utils/log.h"

CHexWorld::CHexWorld() {
	hexRenderer.setCallbackApp(this);
	resolving = false;
	leftMouseHeldDown = false;
	gameTurnActive = false;
}

/** Provide a pointer to the game app to check for mouse input, etc. */
void CHexWorld::setCallbackApp(IhexWorldCallback* pApp) {
	pCallbackApp = pApp;
}

/**	Load a mesh for storage under the given name. */
void CHexWorld::addMesh(const std::string& name, std::vector<CMesh>& meshes) {
	CBuf* meshBuf = hexRenderer.addBuffer(name);
	meshes[0].writeToBufferv3i(*meshBuf);
}

/**	Load a mesh for storage under the given name. */
void CHexWorld::addMesh(const std::string& name, CMesh& mesh) {
	CBuf* meshBuf = hexRenderer.addBuffer(name);
	mesh.writeToBufferv3i(*meshBuf);
}

void CHexWorld::start() {
	createHexObjects();

	createRoom(31,21);
	hexRenderer.setMap(&hexArray);

	hexRenderer.start();	
}

void CHexWorld::keyCheck() {
	if (pCallbackApp->hexKeyNowCallback('W')) {
		hexRenderer.moveCamera({ 0, 1, 0 });
	} 
	else if (pCallbackApp->hexKeyNowCallback('S')) {
		hexRenderer.moveCamera({ 0, -1, 0 });
	} 
	else if (pCallbackApp->hexKeyNowCallback('A')) {
		hexRenderer.moveCamera({ -1,0,0 });
	} 
	else if (pCallbackApp->hexKeyNowCallback('D')) {
		hexRenderer.moveCamera({ 1,0,0 });
	}

	if (pCallbackApp->hexMouseButtonNowCallback(GLFW_MOUSE_BUTTON_LEFT)) {
		if (!resolving && !playerModel.moving && !gameTurnActive) {
			playerModel.startTravel();
		}
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


void CHexWorld::onKeyDown(int key, long mod) {
	
}

void CHexWorld::onMouseButton(int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT &&  action == GLFW_PRESS) {
		if (!resolving) {
			playerModel.startTravel();
		}
		leftMouseHeldDown = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		leftMouseHeldDown = false;
	}
}


void CHexWorld::draw() {
	hexRenderer.draw();
}

void CHexWorld::setAspectRatio(glm::vec2 ratio) {
	hexRenderer.setCameraAspectRatio(ratio);
}

CHexObject* CHexWorld::getCursorObj(){
	return &hexCursor;
}

/** Called every frame to get the hex world up to date.*/
void CHexWorld::update(float dT) {
	resolving = false;
	CHex playerOldHex = playerModel.hexPosition;

	for (auto entity : entities) {
		resolving |= entity->update(dT);
	}

	if (playerModel.hexPosition != playerOldHex) {
		gameTurnActive = true;
	}

	if (gameTurnActive) {
		gameTurn();
	}
	//need to find another way to do this. 
	//A turn shouln't be driven by the player moving one hex
}

/** Provides a callback function for using hexArray's pathfinding facility. */
THexList CHexWorld::getPathCallback(CHex& start, CHex& end) {
	return hexArray.aStarPath(start,end);
}

///////////////////////Private functions/////////////////////////////////

void CHexWorld::createHexObjects() {
	playerModel.buf = hexRenderer.getBuffer("player");
	playerModel.setPosition(0, 0, 0);
	playerModel.setCallbackObj(this);

	robot.buf = hexRenderer.getBuffer("robot");
	robot.setPosition(-5, -5);
	robot.isRobot = true;
	robot.setCallbackObj(this);

	robot2.buf = hexRenderer.getBuffer("robot");
	robot2.setPosition(5, -5);
	robot2.isRobot = true;
	robot2.setCallbackObj(this);

	entities.push_back(&playerModel);
	entities.push_back(&robot);
	entities.push_back(&robot2);

	hexCursor.buf = hexRenderer.getBuffer("cursor");
	hexCursor.setPosition(0, 0, 0);
}

void CHexWorld::createRoom(int w, int h) {
	glm::i32vec2 margin(1);
	glm::i32vec2 boundingBox = glm::i32vec2(w,h) + margin * 2;

	hexArray.init(boundingBox.x, boundingBox.y);

	glm::i32vec2 tL = margin;
	glm::i32vec2 bR = boundingBox - margin;

	for (int y = 0; y < boundingBox.y; y++) {
		for (int x = 0; x < boundingBox.x; x++) {
			if (x < tL.x || x >= bR.x || y < tL.y || y >= bR.y)
				hexArray.getHexOffset(x, y).content = 2;
			else
				hexArray.getHexOffset(x, y).content = 1;
		}
	}

	//create a few walls
	for (int s = 0; s < 8; s++) {
		hexArray.getHexOffset(7 + s, 7).content = 2;
		hexArray.getHexOffset(14, 7+s).content = 2;

		hexArray.getHexOffset(18 + s, 14).content = 2;
		hexArray.getHexOffset(25, 14 + s).content = 2;

	}
	
}

/** Respend to cursor moving to a new hex. */
void CHexWorld::onCursorMove(CHex& mouseHex) {
	setHexCursor(mouseHex);
	playerModel.findTravelPath(hexCursor.hexPosition);	
}


TEntities* CHexWorld::getEntities() {
	return &entities;
}

/** Return the player object's current travel path. */
THexList* CHexWorld::getPlayerPath() {
	return &playerModel.getTravelPath();
}

void CHexWorld::setHexCursor(CHex& pos) {
	hexCursor.setPosition(pos.x,pos.y,pos.z);
}


/** Perform all the actions of a game turn. */
void CHexWorld::gameTurn() {
	//robot default actions - temp!
	if (resolving) {
		return;
	}
		

	robot.findTravelPath(playerModel.hexPosition);
	robot.startTravel();
	

	robot2.findTravelPath(playerModel.hexPosition);
	robot2.startTravel();

	gameTurnActive = false;
}
