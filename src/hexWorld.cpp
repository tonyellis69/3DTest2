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
	meshes[0].writeToBufferv3i(modelBuffers[name]);

	//TO DO: buffers should probably belong to hexRenderer.
	//hexWorld should create the hex objects, asking hexRenderer to supply the named buffer
}

void CHexWorld::start() {
	//create player object
	playerModel.buf = &modelBuffers["test"];
	playerModel.setPosition(-3, -3, 6);

	hexCursor.buf = &modelBuffers["test"];
	hexCursor.setPosition(0, 0, 0);


	hexRenderer.start();
}

void CHexWorld::keyCheck() {


	if (pCallbackApp->hexKeyNowCallback('W')) {
		//camera.elevate(cameraStep);
		hexRenderer.moveCamera({ 0, 1, 0 });
	}
	if (pCallbackApp->hexKeyNowCallback('S')) {
		//camera.elevate(-cameraStep);
		hexRenderer.moveCamera({ 0, -1, 0 });
	}
	if (pCallbackApp->hexKeyNowCallback('A')) {
		hexRenderer.moveCamera({ -1,0,0 });
	}
	if (pCallbackApp->hexKeyNowCallback('D')) {
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

}



void CHexWorld::draw() {
	hexRenderer.draw();
}

void CHexWorld::setAspectRatio(glm::vec2 ratio) {
	hexRenderer.setCameraAspectRatio(ratio);
}

CHexObject* CHexWorld::getCursorObj() {
	return &hexCursor;
}

CHexObject* CHexWorld::getEntity() {
	return &playerModel;
}

void CHexWorld::setHexCursor(CHex& pos) {
	hexCursor.setPosition(pos.x,pos.y,pos.z);
}
