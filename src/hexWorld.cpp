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

	path = hexArray.breadthFirstPath(CHex(0, 0, 0), CHex(12, 0, -12));
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
	setHexCursor(selectedHex);
	
	updateCursorPath();
	
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
	
	//cycle through all entities
	resolving = resolving || playerModel.update(dT);

	//ensure displayed path doesn't jiggle away from travel path
	if (playerModel.moving)
		hexRenderer.setCursorPath(playerModel.getTravelPath());
	//TO DO: travel path and cursor path should ultimately be two different 
	//paths we display

	if (!resolving && leftMouseDown) {
		playerModel.moveOrder();
	}
}

///////////////////////Private functions/////////////////////////////////

void CHexWorld::tmpCreateArray() {
	hexArray.init(40, 40);
	hexArray.getHex(10, 10).content = 2;
}


CHexObject* CHexWorld::getEntity() {
	return &playerModel;
}

void CHexWorld::setHexCursor(CHex& pos) {
	hexCursor.setPosition(pos.x,pos.y,pos.z);
}

void CHexWorld::updateCursorPath() {
	hexRenderer.setCursorPath(playerModel.hexPosition, hexCursor.hexPosition);

}

/** Move the player object to the first hex on the cursor path. */
void CHexWorld::movePlayerDownPath() {

	playerModel.setTravelPath(hexRenderer.getCursorPath());

	playerModel.moveOrder();

}
