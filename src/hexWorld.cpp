#include "hexWorld.h"

#include "utils/log.h"

CHexWorld::CHexWorld() {
	hexRenderer.setCallbackApp(this);
}

void CHexWorld::setCallbackApp(IhexWorldCallback* pApp) {
	pCallbackApp = pApp;
}

/**	Load the line meshes we're going to use. */
void CHexWorld::addMesh(const std::string& name, std::vector<CMesh>& meshes) {
	CBuf* meshBuf = hexRenderer.addBuffer(name);
	meshes[0].writeToBufferv3i(*meshBuf);
}

void CHexWorld::start() {
	//create player object
	playerModel.buf = hexRenderer.getBuffer("test");
	playerModel.setPosition(0, 0, 0);
	playerModel.setZheight(0.05f);

	hexCursor.buf = hexRenderer.getBuffer("test");
	hexCursor.setPosition(0, 0, 0);

	resolving = false;
	leftMouseDown = false;


	tmpCreateArray();
	hexRenderer.setMap(&hexArray);

	hexRenderer.start();


	//path = hexArray.breadthFirstPath(CHex(0, 0, 0), CHex(12, 0, -12));
	//path = hexArray.breadthFirstPath(CHex(0, 0, 0), CHex(0,-12, 12));
	path = hexArray.breadthFirstPath(CHex(0, 0, 0), CHex(-1, 2, -1));
	//path = hexArray.aStarPath(CHex(0, 0, 0), CHex(0, -12, 12));
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
}

void CHexWorld::onMouseWheel(float delta) {
	if (pCallbackApp->hexKeyNowCallback(GLFW_KEY_LEFT_SHIFT)) {
		hexRenderer.pitchCamera(delta);
	}
	else
		hexRenderer.dollyCamera(delta);
}

void CHexWorld::onMouseMove(int x, int y, int key) {
	//update the hex cursor
	CHex selectedHex = hexRenderer.pickHex(x, y);
	if (selectedHex != hexCursor.hexPosition) {
		setHexCursor(selectedHex);
		liveLog << "\n" << selectedHex.getCubeVec();
		if (selectedHex == CHex(5, -1, -4))
			int b = 0;
		updateCursorPath();
	}
	
}


void CHexWorld::onKeyDown(int key, long mod) {
	switch (key) {
	case GLFW_KEY_KP_9:
		playerModel.move(hexNE); break;
	case GLFW_KEY_KP_6:
		playerModel.move(hexEast); break;
	case GLFW_KEY_KP_3:
		playerModel.move(hexSE); break;
	case GLFW_KEY_KP_1:
		playerModel.move(hexSW); break;
	case GLFW_KEY_KP_4:
		playerModel.move(hexWest); break;
	case GLFW_KEY_KP_7:
		playerModel.move(hexNW); break;

	}

	updateCursorPath();
}

void CHexWorld::onMouseButton(int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT &&  action == GLFW_PRESS) {
		if (!resolving)
			movePlayerDownPath();
		leftMouseDown = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		leftMouseDown = false;
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
	
	//cycle through all entities
	resolving = resolving || playerModel.update(dT);



	if (!resolving && leftMouseDown) {
		//hexRenderer.setCursorPath(playerModel.getTravelPath());
		playerModel.moveOrder();
	}


	//ensure displayed path doesn't jiggle away from travel path
	if (playerModel.moving)
		hexRenderer.setCursorPath(playerModel.getTravelPath());
	else
		if (playerModel.hexPosition != playerOldHex)
			updateCursorPath();
	//TO DO: travel path and cursor path should ultimately be two different 
	//paths we display
	//display the travel path OR the cursor path
}

///////////////////////Private functions/////////////////////////////////

void CHexWorld::tmpCreateArray() {
	hexArray.init(40, 40);
	hexArray.getHexOffset(10, 10).content = 2;


	for (int q = -1; q < 9; q++) {
		hexArray.getHexAxial(q, -4).content = 2;
	}
	hexArray.getHexAxial(0, 0).content = 2;
	

	
}


CHexObject* CHexWorld::getEntity() {
	return &playerModel;
}

void CHexWorld::setHexCursor(CHex& pos) {
	hexCursor.setPosition(pos.x,pos.y,pos.z);
}

void CHexWorld::updateCursorPath() {
	THexList path;
	//if (!hexArray.outsideArray(hexCursor.hexPosition) && hexArray.getHexCube(hexCursor.hexPosition).content != 2)
		 path = hexArray.aStarPath(playerModel.hexPosition, hexCursor.hexPosition);
	hexRenderer.setCursorPath(path);
	liveLog << "\npath length " << path.size();
}

/** Move the player object to the first hex on the cursor path. */
void CHexWorld::movePlayerDownPath() {

	playerModel.setTravelPath(hexRenderer.getCursorPath());

	playerModel.moveOrder();

}
