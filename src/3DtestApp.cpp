#include "3DtestApp.h"

#include <glew.h>
#include <algorithm>
#include <string>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/perpendicular.hpp>
#include <glm/gtx/common.hpp>
#include <glm/gtx/rotate_vector.hpp> 

#include "colour.h"

//#include "gameWin.h"

#include "buf2.h"

#include "sound/sound.h"

#include "utils/log.h"

using namespace glm;



C3DtestApp::C3DtestApp() {

	defencePopWin = NULL;
}

void C3DtestApp::onStart() {
	appMode = hexMode;// texGenMode;// terrainMode; 
					  //textMode; //hexMode; //physicsMode;

	/*if (appMode == hexMode)
		logWindow->setTextColour(glm::vec4(1));*/


	dataPath = homeDir + "Data\\";



	fnt::loadFromFile(dataPath + "merri16.fnt", "mainHeaderFnt");
	fnt::loadFromFile(dataPath + "merri16L.fnt", "mainFnt");
	fnt::loadFromFile(dataPath + "merri12.fnt", "smallHeaderFnt");
	fnt::loadFromFile(dataPath + "merri12L.fnt", "smallFnt");



	if (appMode == hexMode) {
		initHexWorld();
	}


	return;
}




/** Called every frame, provides a place for the user to check input where constant feedback is required. */
void C3DtestApp::keyCheck() {
	if (appMode == hexMode) {
		//if (hexWorld.editMode) { //edit-mode keystrokes:
		//	if (keyNow('W')) {

		//		hexWorld.moveCamera(glm::vec3{ 0, 1, 0 });
		//	}
		//	else if (keyNow('S')) {
		//		hexWorld.moveCamera(glm::vec3{ 0, -1, 0 });
		//	}
		//	else if (keyNow('A')) {
		//		hexWorld.moveCamera(glm::vec3{ -1,0,0 });
		//	}
		//	else if (keyNow('D')) {
		//		hexWorld.moveCamera(glm::vec3{ 1,0,0 });
		//	}



		//	return;

		//}







		//in-game key strokes:

		if (keyNow(GLFW_KEY_LEFT_CONTROL)) {
			if (keyNow('W')) {

				hexEngine.moveCamera(glm::vec3{ 0, 1, 0 });
			}
			else if (keyNow('S')) {
				hexEngine.moveCamera(glm::vec3{ 0, -1, 0 });
			}
			else if (keyNow('A')) {
				hexEngine.moveCamera(glm::vec3{ -1,0,0 });
			}
			else if (keyNow('D')) {
				hexEngine.moveCamera(glm::vec3{ 1,0,0 });
			}
		}
		else {

			if (moveKeyDown)
				moveKeyChangeTimer += float(dT);
			
			if (moveKeyChangeTimer < 0.05f)
				return;


			//if (keyNow('A')) {
			//	if (keyNow('W'))
			//		game.player->moveCommand(moveNW);
			//	else if (keyNow('S'))
			//		game.player->moveCommand(moveSW);
			//	else
			//		game.player->moveCommand(moveWest);
			//}
			//else if (keyNow('D')) {
			//	if (keyNow('W'))
			//		game.player->moveCommand(moveNE);
			//	else if (keyNow('S'))
			//		game.player->moveCommand(moveSE);
			//	else
			//		game.player->moveCommand(moveEast);
			//}
			//else if (keyNow('W')) {
			//	game.player->moveCommand(moveNorth);
			//}
			//else if (keyNow('S')) {
			//	game.player->moveCommand(moveSouth);
			//}
		
		}





	}




}



/** Triggered *when* a key is pressed, not while it is held down. This is not 'whileKeyDown'. */
void C3DtestApp::onKeyDown(int key, long mod) {
	if (appMode == hexMode) {
		if (key == GLFW_KEY_F1)
			hexEngine.toggleEditMode();

		if (key == 'W')
			moveKeyDown |= upKey;
		else if (key == 'D')
			moveKeyDown |= rightKey;
		else if (key == 'S')
			moveKeyDown |= downKey;
		else if (key == 'A')
			moveKeyDown |= leftKey;



		if (key == 'V') {
			hexEngine.toggleView();
		}

		if (key == GLFW_KEY_SPACE) {
			Paused = !Paused;
			if (Paused)
				sysLog << "\n!!!!Paused!!!!";
			gameWorld.togglePause();
		}

		if (key == 'Z' && mod == GLFW_MOD_CONTROL) {
			hexEngine.onUndo();
		}
		if (key == 'Y' && mod == GLFW_MOD_CONTROL) {
			hexEngine.onRedo();
		}

		return;
	}




}

void C3DtestApp::onKeyUp(int key, long mod) {
	if (appMode == hexMode) {

		unsigned int prev = moveKeyDown;
		if (key == 'W')
			moveKeyDown &= ~upKey;
		else if (key == 'D')
			moveKeyDown &= ~rightKey;
		else if (key == 'S')
			moveKeyDown &= ~downKey;
		else if (key == 'A')
			moveKeyDown &= ~leftKey;
		
		if (moveKeyDown != prev)
			moveKeyChangeTimer = 0;





		//if (key == 'W' || key == 'S')
		//	world.player->onVerticalKeyRelease();
	}
}





/** Called when mouse moves. */
void C3DtestApp::mouseMove(int x, int y, int key) {

}



void C3DtestApp::draw() {

	if (appMode == hexMode) {
		renderer.setBackColour((rgba&)style::uialmostBlack);
		renderer.clearFrame();
		hexEngine.draw(); 
		return;
	}

}





/** Called every frame. Mainly tells other entities to update. */
void C3DtestApp::Update() {

	if (appMode == hexMode)
		hexEngine.update(float(dT));

}

/** Handle messages from GUI controls. */
void C3DtestApp::GUImsg(int ctrlId, TGUImessage& msg) {
	if (msg.msg == uiMsgHotTextClick)
		liveLog << "\nHot text " << msg.value << " clicked!";
}




void C3DtestApp::HandleUImsg(CGUIbase& control, CMessage& Message) {


	//if (control.parentEntity->getUniqueID() == worldUI.invPanelID && Message.Msg == uiMsgHotTextClick) {
	//	glm::i32vec2 mousePos = glm::i32vec2(Message.x, Message.y);
	//	worldUI.inventoryClick(Message.value, mousePos);
	//	return;
	//}

	////popup menu text click
	//if (control.parentEntity->id == popMenuId && Message.Msg == uiMsgHotTextClick) {
	//	glm::i32vec2 mousePos = control.localToScreenCoords(Message.x, Message.y);
	//	worldUI.menuClick(Message.value, mousePos, (CGUIrichTextPanel*)control.parentEntity);
	//	return;
	//}

	//if (control.parentEntity->id == popMenuId && Message.Msg == uiClickOutside) {
	//	glm::i32vec2 mousePos = control.localToScreenCoords(Message.x, Message.y);
	//	worldUI.deletePopupMenu((CGUIrichTextPanel*)control.parentEntity);
	//	return;
	//}

	//if (control.parentEntity->id == popMenuId && Message.Msg == uiMsgMouseMove) {
	//	worldUI.queueMsg(TvmAppMsg{ appHotTxtChange,"","",Message.value });;
	//	return;
	//}


	//if (control.parentEntity->id == popMenuId && Message.Msg == uiMouseWheel) {
	//	worldUI.mouseWheelHotText(Message.value, Message.value2);
	//	return;
	//}



}


void C3DtestApp::onResize(int width, int height) {
	hexEngine.setAspectRatio(glm::vec2(width, height));
}

void C3DtestApp::addGameWindow(CGUIbase* gameWin) {
	GUIroot.add(gameWin);
}

/** Get the hexWorld ready for use. */
void C3DtestApp::initHexWorld() {
	hexEngine.addMesh("test", dataPath + "models\\test.obj");
	hexEngine.addMesh("cursor", dataPath + "models\\cursor.dae");
	


	//hexWorld.addMesh("player", dataPath + "models\\player.obj");
	hexEngine.addMesh("player", dataPath + "models\\basePlayer.dae");

	hexEngine.addMesh("dummyItem", dataPath + "models\\dummyItem.dae");
	hexEngine.addMesh("medkit", dataPath + "models\\medkit.dae");

	//hexWorld.addMesh("robot", dataPath + "models\\robot.obj");
	hexEngine.addMesh("robot", dataPath + "models\\robot.dae");
	hexEngine.addMesh("shield", dataPath + "models\\shield.obj");
	hexEngine.addMesh("bolt", dataPath + "models\\bolt.obj");
	hexEngine.addMesh("desk", dataPath + "models\\desk.obj");

	hexEngine.addMesh("door", dataPath + "models\\door.dae");

	hexEngine.addMesh("solidHex", dataPath + "models\\solidHex.dae");

	hexEngine.addMesh("gun", dataPath + "models\\gun.dae");

	hexEngine.addMesh("armour", dataPath + "models\\armour.dae");

	hexEngine.addMesh("reticule", dataPath + "models\\reticule.dae");
	hexEngine.addMesh("hex", dataPath + "models\\hex.dae");

	//... more models


	//tiles
	std::vector<glm::vec4> hex = { { 0.0f,0.3f,0.0f,1.0f} };
	std::vector<glm::vec4> largerHex = { { 0.0f,0.0f,0.9f,1.0f}, { 0.5f,0.5f,0.8f,1.0f} };
	std::vector<glm::vec4> green = { { 0.0f,0.9f,0.0f,1.0f} };
	std::vector<glm::vec4> red = { { 0.9f,0.0f,0.0f,1.0f} };

	hexEngine.addHexTile("hex", dataPath + "models\\hex.dae", hex);
	hexEngine.addHexTile("largeHex", dataPath + "models\\largeHex.dae", largerHex);
	hexEngine.addHexTile("dummy", dataPath + "models\\dummyItem.dae", largerHex); //temp for test purposes!
	hexEngine.addHexTile("dummy2", dataPath + "models\\dummyItem.dae", green); //temp for test purposes!
	hexEngine.addHexTile("dummy3", dataPath + "models\\dummyItem.dae", red); //temp for test purposes!

	hexEngine.initialise(); //TODO: maybe the meshloading above goes in init? Is one-off setup stuff.
	//hexWorld.start();

}

glm::i32vec2 C3DtestApp::getMousePos() {
	return CBaseApp::getMousePos();
}




/** Trap mousewheel events for our own use. */
bool C3DtestApp::OnMouseWheelMsg(float xoffset, float yoffset) {
	int x, y;
	win.getMousePos(x, y);
	int delta = int(yoffset);
	int keyState = 0; //can get key state if ever needed

	bool handled = CBaseApp::OnMouseWheelMsg(xoffset, yoffset);

	if (!handled) {
		if (appMode == hexMode) { //kill this, handled by hexWorld directly
			if (keyNow(GLFW_KEY_LEFT_SHIFT))
				keyState = GLFW_KEY_LEFT_SHIFT;
			if (keyNow(GLFW_KEY_LEFT_CONTROL))
				keyState = GLFW_KEY_LEFT_CONTROL;
			if (keyNow(GLFW_KEY_LEFT_ALT))
				keyState = GLFW_KEY_LEFT_ALT;
			//hexWorld.onMouseWheel(yoffset,keyState);
			handled = true; //cheeky
		}
	}
	return handled;
}
//
//void C3DtestApp::onPopupText(CSendText& msg) {
//	if (msg.popupType == defencePopup) {
//		if (defencePopWin == NULL) {
//			defencePopWin = new CGameTextWin();
//			defencePopWin->setLocalPos(100,style::mainWinCtrlBorder);
//		//	defencePopWin->hFormat = hCentre;
//			defencePopWin->resize(style::defPopupW, style::defPopupH);
//			//defencePopWin->anchorBottom = style::mainWinCtrlBorder;
//			defencePopWin->setVisible(false);
//			//defencePopWin->setTheme("smallNormal");
//			hexWorld.subscribe(defencePopWin);
//			GUIroot.add(defencePopWin);
//		}
//		defencePopWin->clearText();
//		defencePopWin->addText(msg.text);
//		defencePopWin->setVisible(true);
//	}
//	else if (msg.popupType == statusPopup) {
//		if (statusPopWin == NULL) {
//			statusPopWin = new CGameTextWin();
//			statusPopWin->setLocalPos(100, style::mainWinCtrlBorder + 20);
//			//statusPopWin->hFormat = hCentre;
//			statusPopWin->resize(style::defPopupW, style::defPopupH);
//			statusPopWin->anchorBottom = style::mainWinCtrlBorder;
//			statusPopWin->setVisible(false);
//			//statusPopWin->setTheme("smallNormal");
//			hexWorld.subscribe(statusPopWin);
//			GUIroot.add(statusPopWin);
//		}
//		statusPopWin->clearText();
//		statusPopWin->addText(msg.text);
//		statusPopWin->setVisible(true);
//	}
//	//else if (msg.popupType == powerQ) {
//	//	/*powerQueueWin->clearText();
//	//	powerQueueWin->addText(msg.text);*/
//	//}
//	//else if (msg.popupType == combatLog) {
//	//	if (msg.clear)
//	//		combatLogWin->clearText();
//	//	else
//	//		combatLogWin->addText(msg.text);
//	//}
//}

/** Handle generic system messages .*/
//void C3DtestApp::onSysMessage(CSysMsg& msg) {
//	;// powerQueueWin->setVisible(msg.isOn);
//}



C3DtestApp::~C3DtestApp() {

}





