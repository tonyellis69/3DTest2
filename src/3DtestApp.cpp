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

	vm.loadProgFile(dataPath + "..\\..\\TC\\Debug\\output.tig");

	if (appMode == hexMode) {
		initHexWorld();
		makePowerQueueWin();
		makeCombatLogWin();
	}


	messageBus.setHandler<CSendText>(this, &C3DtestApp::onPopupText); 
	messageBus.setHandler<CSysMsg>(this, &C3DtestApp::onSysMessage);

	return;
}




/** Called every frame, provides a place for the user to check input where constant feedback is required. */
void C3DtestApp::keyCheck() {
	if (appMode == hexMode) {
		if (hexWorld.editMode) { //edit-mode keystrokes:
			if (keyNow('W')) {

				hexWorld.moveCamera(glm::vec3{ 0, 1, 0 });
			}
			else if (keyNow('S')) {
				hexWorld.moveCamera(glm::vec3{ 0, -1, 0 });
			}
			else if (keyNow('A')) {
				hexWorld.moveCamera(glm::vec3{ -1,0,0 });
			}
			else if (keyNow('D')) {
				hexWorld.moveCamera(glm::vec3{ 1,0,0 });
			}



			return;

		}







		//in-game key strokes:

		if (keyNow(GLFW_KEY_LEFT_CONTROL)) {
			if (keyNow('W')) {

				hexWorld.moveCamera(glm::vec3{ 0, 1, 0 });
			}
			else if (keyNow('S')) {
				hexWorld.moveCamera(glm::vec3{ 0, -1, 0 });
			}
			else if (keyNow('A')) {
				hexWorld.moveCamera(glm::vec3{ -1,0,0 });
			}
			else if (keyNow('D')) {
				hexWorld.moveCamera(glm::vec3{ 1,0,0 });
			}
		}
		else {

			if (moveKeyDown)
				moveKeyChangeTimer += dT;
			
			if (moveKeyChangeTimer < 0.05f)
				return;


			if (keyNow('A')) {
				if (keyNow('W'))
					game.player->moveCommand(moveNW);
				else if (keyNow('S'))
					game.player->moveCommand(moveSW);
				else
					game.player->moveCommand(moveWest);
			}
			else if (keyNow('D')) {
				if (keyNow('W'))
					game.player->moveCommand(moveNE);
				else if (keyNow('S'))
					game.player->moveCommand(moveSE);
				else
					game.player->moveCommand(moveEast);
			}
			else if (keyNow('W')) {
				game.player->moveCommand(moveNorth);
			}
			else if (keyNow('S')) {
				game.player->moveCommand(moveSouth);
			}

		}




	}




}



/** Triggered *when* a key is pressed, not while it is held down. This is not 'whileKeyDown'. */
void C3DtestApp::onKeyDown(int key, long mod) {
	if (appMode == hexMode) {
		if (key == GLFW_KEY_F1)
			hexWorld.toggleEditMode();

		if (key == 'W')
			moveKeyDown |= upKey;
		else if (key == 'D')
			moveKeyDown |= rightKey;
		else if (key == 'S')
			moveKeyDown |= downKey;
		else if (key == 'A')
			moveKeyDown |= leftKey;



		if (key == GLFW_KEY_ENTER && !hexWorld.editMode)
			hexWorld.enterKeyDown();

		if (key == 'V') {
			hexWorld.toggleView();
		}

		if (key == GLFW_KEY_SPACE) {
			Paused = !Paused;
			game.togglePause();
		}

		if (key == 'Z' && mod == GLFW_MOD_CONTROL) {
			hexWorld.onUndo();
		}
		if (key == 'Y' && mod == GLFW_MOD_CONTROL) {
			hexWorld.onRedo();
		}

		hexWorld.onKeyDown(key, mod);
		return;
	}

	if (key == 'R' && mod == GLFW_MOD_CONTROL) {
		if (appMode == textMode) {
			vm.reset();
			vm.reloadProgFile();
			//worldUI.reset();
			return;
		}



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

		hexWorld.onKeyUp(key, mod);



		//if (key == 'W' || key == 'S')
		//	world.player->onVerticalKeyRelease();
	}
}


void C3DtestApp::onMouseButton(int button, int action, int mods) {
	if (appMode == hexMode) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS)
				hexWorld.onFireKey(true,mods);
			else
				hexWorld.onFireKey(false,mods);

			


		}


		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)
				hexWorld.onRightKey(true, mods);
			else
				hexWorld.onRightKey(false, mods);
		}

	}
};


/** Called when mouse moves. */
void C3DtestApp::mouseMove(int x, int y, int key) {
	if (appMode == hexMode)
		hexWorld.onMouseMove(x, y, key);
}



void C3DtestApp::draw() {

	if (appMode == hexMode) {
		renderer.setBackColour((rgba&)style::uialmostBlack);
		renderer.clearFrame();
		hexWorld.draw();
		return;
	}

}





/** Called every frame. Mainly tells other entities to update. */
void C3DtestApp::Update() {

	vmUpdate();

	if (appMode == hexMode)
		hexWorld.update(float(dT));

}

/** Handle messages from GUI controls. */
void C3DtestApp::GUImsg(int ctrlId, TGUImessage& msg) {
	if (msg.msg == uiMsgHotTextClick)
		liveLog << "\nHot text " << msg.value << " clicked!";
}



/** Handle messages from the virtual machine. */
void C3DtestApp::vmMessage(TvmAppMsg msg) {
	if (msg.type == appFlush) {
		;// worldUI.flushMessageQueue();
		return;
	}

	;// worldUI.queueMsg(msg);
}

/** Carry out any processing demanded by the virtual machine. */
void C3DtestApp::vmUpdate() {

	/*if (vm.getStatus() == vmAwaitChoice && !shownChoice) {
		showChoice();
		//getChoice(vm);
	} */
	if (vm.getStatus() == vmAwaitString) {
		//getString(vm);
	}
	if (vm.getStatus() == vmExecuting) {
		vm.execute();
	}

}




void C3DtestApp::HandleUImsg(CGUIbase& control, CMessage& Message) {


	//if (control.parent->getUniqueID() == worldUI.invPanelID && Message.Msg == uiMsgHotTextClick) {
	//	glm::i32vec2 mousePos = glm::i32vec2(Message.x, Message.y);
	//	worldUI.inventoryClick(Message.value, mousePos);
	//	return;
	//}

	////popup menu text click
	//if (control.parent->id == popMenuId && Message.Msg == uiMsgHotTextClick) {
	//	glm::i32vec2 mousePos = control.localToScreenCoords(Message.x, Message.y);
	//	worldUI.menuClick(Message.value, mousePos, (CGUIrichTextPanel*)control.parent);
	//	return;
	//}

	//if (control.parent->id == popMenuId && Message.Msg == uiClickOutside) {
	//	glm::i32vec2 mousePos = control.localToScreenCoords(Message.x, Message.y);
	//	worldUI.deletePopupMenu((CGUIrichTextPanel*)control.parent);
	//	return;
	//}

	//if (control.parent->id == popMenuId && Message.Msg == uiMsgMouseMove) {
	//	worldUI.queueMsg(TvmAppMsg{ appHotTxtChange,"","",Message.value });;
	//	return;
	//}


	//if (control.parent->id == popMenuId && Message.Msg == uiMouseWheel) {
	//	worldUI.mouseWheelHotText(Message.value, Message.value2);
	//	return;
	//}



}


void C3DtestApp::onResize(int width, int height) {
	hexWorld.setAspectRatio(glm::vec2(width, height));
}

void C3DtestApp::addGameWindow(CGUIbase* gameWin) {
	GUIroot.add(gameWin);
}

/** Get the hexWorld ready for use. */
void C3DtestApp::initHexWorld() {
	hexWorld.setVM(&vm);

	hexWorld.addMesh("test", dataPath + "models\\test.obj");
	hexWorld.addMesh("cursor", dataPath + "models\\cursor.dae");
	//hexWorld.addMesh("player", dataPath + "models\\player.obj");
	hexWorld.addMesh("player", dataPath + "models\\basePlayer.dae");

	hexWorld.addMesh("dummyItem", dataPath + "models\\dummyItem.dae");
	hexWorld.addMesh("medkit", dataPath + "models\\medkit.dae");

	//hexWorld.addMesh("robot", dataPath + "models\\robot.obj");
	hexWorld.addMesh("robot", dataPath + "models\\robot.dae");
	hexWorld.addMesh("shield", dataPath + "models\\shield.obj");
	hexWorld.addMesh("bolt", dataPath + "models\\bolt.obj");
	hexWorld.addMesh("desk", dataPath + "models\\desk.obj");

	hexWorld.addMesh("door", dataPath + "models\\door.dae");

	hexWorld.addMesh("solidHex", dataPath + "models\\solidHex.dae");





	//... more models



	ITigObj* tigMapTemplate = vm.getObject("testRoom");
	hexWorld.makeMap(tigMapTemplate);
	hexWorld.startGame();
}

glm::i32vec2 C3DtestApp::getMousePos() {
	return CBaseApp::getMousePos();
}

void C3DtestApp::makePowerQueueWin() {
	//powerQueueWin = new CGameTextWin();
	//powerQueueWin->setLocalPos(100, style::mainWinCtrlBorder + 10);
	//powerQueueWin->setSize(style::powerQueueWinSize);
	//powerQueueWin->anchorRight = style::mainWinCtrlBorder;
	//powerQueueWin->setTheme("smallNormal");
	//GUIroot.add(powerQueueWin);
}

void C3DtestApp::makeCombatLogWin() {
	//combatLogWin = new CGameTextWin();
	//combatLogWin->setLocalPos(style::mainWinCtrlBorder, style::mainWinCtrlBorder + 20);
	//combatLogWin->setSize(style::combatLogWinSize);
	//combatLogWin->setTheme("smallNormal");
	//GUIroot.add(combatLogWin);
}


/** Trap mousewheel events for our own use. */
bool C3DtestApp::OnMouseWheelMsg(float xoffset, float yoffset) {
	int x, y;
	win.getMousePos(x, y);
	int delta = int(yoffset);
	int keyState = 0; //can get key state if ever needed

	bool handled = CBaseApp::OnMouseWheelMsg(xoffset, yoffset);

	if (!handled) {
		if (appMode == hexMode) {
			if (keyNow(GLFW_KEY_LEFT_SHIFT))
				keyState = GLFW_KEY_LEFT_SHIFT;
			if (keyNow(GLFW_KEY_LEFT_CONTROL))
				keyState = GLFW_KEY_LEFT_CONTROL;
			if (keyNow(GLFW_KEY_LEFT_ALT))
				keyState = GLFW_KEY_LEFT_ALT;
			hexWorld.onMouseWheel(yoffset,keyState);
			handled = true; //cheeky
		}
	}
	return handled;
}

void C3DtestApp::onPopupText(CSendText& msg) {
	if (msg.popupType == defencePopup) {
		if (defencePopWin == NULL) {
			defencePopWin = new CGameTextWin();
			defencePopWin->setLocalPos(100,style::mainWinCtrlBorder);
		//	defencePopWin->hFormat = hCentre;
			defencePopWin->resize(style::defPopupW, style::defPopupH);
			//defencePopWin->anchorBottom = style::mainWinCtrlBorder;
			defencePopWin->setVisible(false);
			//defencePopWin->setTheme("smallNormal");
			hexWorld.subscribe(defencePopWin);
			GUIroot.add(defencePopWin);
		}
		defencePopWin->clearText();
		defencePopWin->addText(msg.text);
		defencePopWin->setVisible(true);
	}
	else if (msg.popupType == statusPopup) {
		if (statusPopWin == NULL) {
			statusPopWin = new CGameTextWin();
			statusPopWin->setLocalPos(100, style::mainWinCtrlBorder + 20);
			//statusPopWin->hFormat = hCentre;
			statusPopWin->resize(style::defPopupW, style::defPopupH);
			statusPopWin->anchorBottom = style::mainWinCtrlBorder;
			statusPopWin->setVisible(false);
			//statusPopWin->setTheme("smallNormal");
			hexWorld.subscribe(statusPopWin);
			GUIroot.add(statusPopWin);
		}
		statusPopWin->clearText();
		statusPopWin->addText(msg.text);
		statusPopWin->setVisible(true);
	}
	//else if (msg.popupType == powerQ) {
	//	/*powerQueueWin->clearText();
	//	powerQueueWin->addText(msg.text);*/
	//}
	//else if (msg.popupType == combatLog) {
	//	if (msg.clear)
	//		combatLogWin->clearText();
	//	else
	//		combatLogWin->addText(msg.text);
	//}
}

/** Handle generic system messages .*/
void C3DtestApp::onSysMessage(CSysMsg& msg) {
	;// powerQueueWin->setVisible(msg.isOn);
}



C3DtestApp::~C3DtestApp() {

}





