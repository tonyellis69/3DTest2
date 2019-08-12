#include "worldUI.h"
#include "worldUI.h"
#include "worldUI.h"

#include <ctype.h>

#include "3DtestApp.h"


CWorldUI::CWorldUI() {
	
}

void CWorldUI::setVM(CTigVM * vm) {
	pVM = vm;
}


void CWorldUI::setGameApp(C3DtestApp * app) {
	pApp = app;
}

/** Any initialisation that has to wait for items to be ready. */
void CWorldUI::init() {
	transcript.setFile(pApp->homeDir + "transcript.txt");
	createTextStyles();

	pVM->execute(); //for any global variables or code
	playerId = pVM->getGlobalVar("playerObj").getObjId();
	clickId = pVM->getMemberId("click");
	examId = pVM->getMemberId("examine");
	moveToId = pVM->getMemberId("moveTo");
	showPlayerOpsId = pVM->getMemberId("showPlayerOptions");

	createMainWindow();
	createInventoryWindow();
	pMenuWindow = NULL; //TO DO: should probably create and make invisible to be consistent
	currentVariant = 0;
}


/** Start a game session. */
void CWorldUI::start() {
	pVM->callMember(zeroObject, "init"); 	//execute Tig initialisation code
}


bool CWorldUI::writeText(std::string & text, int window) {
	if (window == mainWin) {
		if (mainTextPanel->busy())
			return false;
		mainTextPanel->displayText(text);
	}
	else if (window == invWin)
		invPanel->displayText(text);
	else if (window == menuWin) {
		pMenuWindow->displayText(text);
		pMenuWindow->resizeToFit();
		positionWindow(pMenuWindow, lastMouseOverPos);
		
	}
	else { 
		for (auto popupWin : popupWindows) {
			if (popupWin.objId == window) {
				popupWin.win->displayText(text);
				popupWin.win->resizeToFit();
				popupWin.win->setStatus(CGUIrichTextPanel::readyToPosition);
				//TO DO: might want to wait for a vm message before commiting to readyToPosition, we'll see
				return true;
			}
		}
	}
	return true;
}


/**	Player has clicked on the identified hot text in the main window, so
	execute the function call stored for it. */
void CWorldUI::mainWindowClick(unsigned int hotId, glm::i32vec2 mousePos) {
	playerTurn(hotId);
}

/** Player has right-clicked on the main window, so get the game to pop-up
	the handy menu. */
void CWorldUI::mainWindowRightClick(glm::i32vec2 mousePos) {
	lastMouseOverPos = mousePos;
	pVM->callMember(NULL, "shortcutMenu");
}


/** Handle a click on an inventory window item. */
//TO DO: probably redundant, as we no longer click on inventory items but their popups
void  CWorldUI::inventoryClick(unsigned int hotId, glm::i32vec2 mousePos) {
	lastMouseOverPos = mousePos;
	playerTurn(hotId);
}

/**	Carry out one complete turn of the game. This consists of anything that happens
	immediately before the player's action, the player's action itself (and any
	reactions), and then any game activity. */
void CWorldUI::playerTurn(unsigned int actionHotId) {
	TFnCall fnCall = pVM->getHotTextFnCall(actionHotId, currentVariant);
	if (fnCall.msgId != showPlayerOpsId && fnCall.msgId != clickId)
		mainTextPanel->removeMarked();
	mainTextPanel->unhotDuplicates();

	//mainTextPanel->solidifyTempText();
	queueMsg(TvmAppMsg{ appSolidifyTmpText });

	//TO DO: push a copy of fnCall to an array to record player activity for playback

	pVM->callMember(fnCall.objId, fnCall.msgId,fnCall.params);

	//instantaneous actions don't end the player's turn
	if (fnCall.msgId == clickId || fnCall.msgId == examId || fnCall.msgId == showPlayerOpsId )
		return; 

	pVM->callMember(NULL, "gameTurn");
}

/** Register change in current room. */
void CWorldUI::handleRoomChange(int roomId) {
	//TO DO: ever doing anything here?
}

void CWorldUI::openWindow(int winId, bool modal) {
	if (winId == menuWin)
		openMenuWindow(winId);
	else
		spawnPopupWindow(winId);
}

void CWorldUI::openMenuWindow(int winId) {
	pMenuWindow = spawnPopText(true);
	pMenuWindow->setTextStyle("small");
	pMenuWindow->setResizeMode(resizeByWidthMode);
	pMenuWindow->setLocalPos(lastMouseOverPos.x, lastMouseOverPos.y );
	pMenuWindow->id = popMenuId;
}



/** Create a pop-up window with the given id. */
void CWorldUI::spawnPopupWindow(int objId) {
	for (auto popWindow : popupWindows) {
		if (popWindow.objId == objId)
			return;
	}
	CGUIrichTextPanel* pop = spawnPopText(true);
	pop->draggable = true;

	pop->setLocalPos(lastMouseOverPos.x, lastMouseOverPos.y);
	pop->resize(300, 200);
	pop->setResizeMode(resizeByRatioMode);
	pop->id = popObjWinId;
	pop->uniqueID = objId; //TO DO: should really be id, try to phase out the above
	popupWindows.push_back({ pop,objId });

}


void CWorldUI::purgeMainPanel(unsigned int id) {
	vector<unsigned int> purgedIds = mainTextPanel->purgeHotText(id);
	//whatever hot text was removed, remove also from the list of hot text function calls.
	for (auto purgedId : purgedIds)
		pVM->removeHotTextFnCall(purgedId);
}

bool CWorldUI::clearWindow(int window) {
	if (window == mainWin) {
		clearWindowHotIds(mainTextPanel);
		mainTextPanel->clear();
		return true;
	}
	if (window == invWin) {
		clearWindowHotIds(invPanel);
		invPanel->clear();
		return true;
	}

	for (auto popjWin : popupWindows)
		if (popjWin.objId == window) {
			clearWindowHotIds(popjWin.win);
			popjWin.win->clear();
			return true;
		}


	return false;
}

void CWorldUI::clearMarkedText(int window) {
	if (window == mainWin) {
		mainTextPanel->removeMarked();
	}
}

void CWorldUI::clearWindowHotIds(CGUIrichTextPanel* panel) {
	vector<unsigned int> lostIds = panel->richText->getHotTextIds();
	for (auto hotId : lostIds) {
		pVM->removeHotTextFnCall(hotId);
	}
}


/** Display the popup menu at the cursor position, adjusted for its dimensions and the screen edge. */
void CWorldUI::positionWindow(CGUIrichTextPanel* popControl, const glm::i32vec2& tlCornerPos) {
	

	int margin = 0; // 20;

	glm::i32vec2 tlCorner = tlCornerPos;
	//glm::i32vec2 tlCorner = popControl->getLocalPos();
	//tlCorner.y -= popControl->getSize().y;

	glm::i32vec2 size =  popControl->getSize() + glm::i32vec2(margin);

	glm::i32vec2 brCornerApp = popControl->parent->getSize();

	tlCorner = glm::min<>(tlCorner, brCornerApp - size);

	popControl->setLocalPos(tlCorner.x, tlCorner.y - size.y);
	popControl->setVisible(true);
}

/** Respond to the user selecting an item from the popup menu. */
void CWorldUI::menuClick(unsigned int hotId, glm::i32vec2& mousePos, CGUIrichTextPanel* popUp) {
	lastMouseOverPos = mousePos;
	playerTurn(hotId);
	deletePopupMenu(popUp);
}

void CWorldUI::deletePopupMenu(CGUIrichTextPanel* popUp) {
	if (popUp == pMenuWindow)
		pMenuWindow = NULL;
	clearWindowHotIds(popUp);
	delete popUp;
}

/** Respond to user clicking on an object window.*/
void CWorldUI::objWindowClick(unsigned int hotId, glm::i32vec2 mousePos, CGUIrichTextPanel * popUp) {
	lastMouseOverPos = mousePos;
	playerTurn(hotId);
	closeObjWindow(popUp);
}


void CWorldUI::objWindowRightClick(CGUIrichTextPanel* popUp){

}

void CWorldUI::closeObjWindow(CGUIrichTextPanel * popUp) {
	clearWindowHotIds(popUp);
	for (unsigned int x = 0; x < objWindows.size(); x++) {
		if (objWindows[x].win == popUp) {
			objWindows.erase(objWindows.begin() + x);
			break;
		}
	}
	delete popUp;
}


void CWorldUI::deletePopupWindow(int id) { 
	for (auto popupWin = popupWindows.begin(); popupWin != popupWindows.end(); popupWin++) {
		if (popupWin->objId == id) {
			clearWindowHotIds(popupWin->win);
			delete popupWin->win;
			popupWindows.erase(popupWin);
			return;
		}
	}
}


void CWorldUI::vmMessage(int p1, int p2) {
	if (p1 == msgRoomChange)
		handleRoomChange(p2);
}

/** Add the given message to the queue for eventual processing. */
void CWorldUI::queueMsg(TvmAppMsg& msg) {
	messages.push(msg);
}

/** Create the main text window. */
void CWorldUI::createMainWindow() {
	mainTextPanel = new CGUIrichTextPanel(200, 50, 800, 700);
	mainTextPanel->setBackColour1(uiWhite);
	mainTextPanel->setBackColour2(uiWhite);
	
	mainTextPanel->setBorderOn(true);
	mainTextPanel->hFormat = hCentre;
	mainTextPanel->setInset(10);
	mainTextPanel->setTextColour(UIwhite);
	mainTextPanel->setResizeMode(resizeByWidthMode);
	mainTextWindowID = mainTextPanel->getID();
	mainTextPanel->setTextStyles(&normalTheme.styles);
	mainTextPanel->setTextStyle("mainBody");
	mainTextPanel->setDefaultTextStyle("mainBody");
	pApp->GUIroot.Add(mainTextPanel);
	mainTextPanel->richText->transcriptLog = &transcript;
	mainTextPanel->deliveryMode = byCharacter;// byClause;
	mainTextPanel->setLineFadeIn(false);
	mainTextPanel->setGUIcallback(this);
}

void CWorldUI::createInventoryWindow() {
	invPanel = new CGUIrichTextPanel(1100, 120, 180, 300);
	invPanel->setBackColour1(white);
	invPanel->setBackColour2(white);
	
	invPanel->setBorderOn(true);
	invPanel->anchorRight = 10;
	invPanel->hFormat = hRight;
	invPanel->setInset(10);
	invPanel->setTextColour(UIwhite);
	invPanel->setResizeMode(resizeByWidthMode);
	invPanelID = invPanel->getID();
	invPanel->setTextStyles(&smallNormalTheme.styles);
	invPanel->setTextStyle("smallHeader");
	invPanel->displayText("Inventory:");
	pApp->GUIroot.Add(invPanel);
	invPanel->setGUIcallback(this);
}


CGUIrichTextPanel* CWorldUI::spawnPopText(bool modal) {
	CGUIrichTextPanel* popupPanel = new CGUIrichTextPanel(0, 0, 300, 300);
	popupPanel->setBackColour1(white);
	popupPanel->setBackColour2(white);
	popupPanel->setBorderOn(true);
	popupPanel->setFont(&pApp->popFont);
	popupPanel->setTextColour(UIwhite);
	popupPanel->setResizeMode(resizeByWidthMode);
	popupPanel->setShortestSpaceBreak(50);
	popupTextID = popupPanel->getRichTextID();
	popupPanelID = popupPanel->getID();
	popupPanel->setGUIcallback(this);
	popupPanel->setVisible(false);
	popupPanel->setTextStyles(&smallNormalTheme.styles);
	//if (modal)
	//	pApp->GUIroot.addModal(popupPanel);
	//else
		pApp->GUIroot.Add(popupPanel);
	return popupPanel;
}

/** Create the various text styles the various text controls use. */
void CWorldUI::createTextStyles() {
	darkGray = glm::vec4(0.25, 0.25, 0.25, 1);
	choiceColour = glm::vec4(0.5, 0.5, 0.5, 1);
	white = glm::vec4(1);
	hottextColour =  glm::vec4(0.28, 0.28, 0.47, 1);
	hottextSelectedColour =  glm::vec4(1, 0.547, 0.0, 1);

	normalTheme.styles.push_back({ "mainBody", "main", darkGray });
	normalTheme.styles.push_back({ "mainHeader","mainHeader",darkGray });
	normalTheme.styles.push_back({ "hot", "main", hottextColour });
	normalTheme.styles.push_back({ "hotSelected", "main", hottextSelectedColour });
	normalTheme.styles.push_back({ "choice", "main", choiceColour });

	smallNormalTheme.styles.push_back({ "small","small",darkGray });
	smallNormalTheme.styles.push_back({ "smallHeader","smallHeader",darkGray });
	smallNormalTheme.styles.push_back({ "hot", "small", hottextColour });
	smallNormalTheme.styles.push_back({ "hotSelected", "small", hottextSelectedColour });

}

void CWorldUI::hide(bool onOff) {
	invPanel->setVisible(!onOff);
	mainTextPanel->setVisible(!onOff);
}

void CWorldUI::reset() {
	pVM->execute(); //for any global variables or code
	playerId = pVM->getGlobalVar("playerObj").getObjId();

	invPanel->clear();
	mainTextPanel->clear();

	start();
}

/** Turn temporary text mode on or off. */
//TO DO: this should work for any window, not just main. 
void CWorldUI::tempText(bool onOff, int winId) {
	mainTextPanel->setTempText(onOff);
}

void CWorldUI::update(float dT) {
	processMessageQueue();

	mainTextPanel->update(dT);
	invPanel->update(dT);
	if (pMenuWindow)
		pMenuWindow->update(dT);

	for (auto& popupWindow : popupWindows) {
		if (popupWindow.win->status == CGUIrichTextPanel::displaying) {
			popupWindow.lifeTime += dT;
			if (popupWindow.lifeTime > 0.15f && popupWindow.win->noMouse() &&
				currentMouseOverObj != popupWindow.objId) {
				deletePopupWindow(popupWindow.objId);
			}
		}
		if (popupWindow.win->status == CGUIrichTextPanel::readyToDelete) {
			deletePopupWindow(popupWindow.objId);
		}
		if (popupWindow.win->status == CGUIrichTextPanel::readyToPosition) {
			positionPopupWindow(popupWindow.win);
			popupWindow.win->setVisible(true);
		}
	}

}

/** Handle a pause request from the vm. Suspend most activity if we're pausing, but leave the
	player able to un-pause/. */
void CWorldUI::pause(bool isOn) {
	if (isOn) { 
		invPanel->suspend(true);
	} 
	else {
		invPanel->suspend(false);
	}
}

/** The player has moused over new hot text, so display the associated text. */
void CWorldUI::mouseOverHotText( int hotId) {
	currentVariant = 0;
	mainTextPanel->collapseTempText();
	if (hotId < 1)
		return;
	displayNarrativeChoice(hotId);
}

void CWorldUI::displayNarrativeChoice(int hotId) {
	TFnCall fnCall = pVM->getHotTextFnCall(hotId, currentVariant);
	if (!fnCall.params.empty()) {
		CTigVar finalParam = fnCall.params.back();
		if (finalParam.type == tigString) {
			mainTextPanel->setTempText(true);
			mainTextPanel->setTextStyle("fadeOn");
			mainTextPanel->deliveryMode = noDelivery;
			string narrativeChoice = finalParam.getStringValue();
			for (int x = 0; x < narrativeChoice.size(); x++) { //catch any hot text and suspend it
				if (narrativeChoice[x] == '\\' && narrativeChoice[x + 1] == 'h')
					narrativeChoice[x + 1] = 'S';
			}
			mainTextPanel->setTextStyle("choice");
			
			mainTextPanel->displayText("\n\n" + narrativeChoice);
			mainTextPanel->deliveryMode = byCharacter;//byClause;
			mainTextPanel->setTextStyle("fadeOff");
			mainTextPanel->setTempText(false);
			mainTextPanel->setTextStyle("mainBody");
		}
	}

}

/** Player is mousewheeling over hot text. Cycle through the variants if they exist. */
void CWorldUI::mouseWheelHotText(int hotId, int direction) {
	THotTextFnCall fnCallrec = pVM->getHotTextFnCallRec(hotId);
	if (fnCallrec.options.size() == 1)
		return;
	currentVariant += direction;
	if (currentVariant >= (int)fnCallrec.options.size())
		currentVariant = 0;
	if (currentVariant < 0)
			currentVariant = fnCallrec.options.size() - 1;
	mainTextPanel->collapseTempText();
	displayNarrativeChoice(hotId);
}



/** Return a semi-random coordinate. */
glm::i32vec2 CWorldUI::randomWindowPos() {
	glm::i32vec2 appSize(pApp->viewWidth, pApp->viewHeight);

	std::uniform_int_distribution<> randomYRange{ 0,int(appSize.y * 0.1f) };
	std::uniform_int_distribution<> randomXRange{ 0,int(appSize.x * 0.75f) };
	int randomY = randomYRange(randEngine);
	int randomX = randomXRange(randEngine);

	return glm::i32vec2(randomX,randomY);
}

/** Handle whatever instructions are currently on the queue. */
void CWorldUI::processMessageQueue() {
	bool result = true;
	while (!messages.empty()) {
		TvmAppMsg& msg = messages.front();
		//TO DO, check if possible, if not bail.
		switch (msg.type) {
			case appWriteText: result = writeText(msg.text, msg.integer); break;
			case appClearWin: result = clearWindow(msg.integer); break;
			case appPurge: purgeMainPanel(msg.integer); break;
			case appOpenWin: openWindow(msg.integer, false); break;
			case appOpenWinModal: openWindow(msg.integer, true); break;
			case appMsg: vmMessage(msg.integer, msg.integer2); break;
			case appTempTxt: tempText((bool)msg.integer, msg.integer2); break;
			case appPause: pause(true); break;
			case appUnpause: pause(false); break;
			case appClearMarked: clearMarkedText(msg.integer); break;
			case appMouseoverHotTxt: mouseOverHotText(msg.integer); break;
			case appSolidifyTmpText : result = mainTextPanel->solidifyTempText();
		}

		if (result)
			messages.pop();
		else
			break;
	}
}

/** First calling point for callbacks from GUI controls such as the rich text panels. */
void CWorldUI::GUIcallback(CGUIbase* sender, CMessage& msg) {
	
	if (msg.Msg == uiMsgHotTextMouseOver) {
		currentMouseOverObj = -1;
		if (msg.value != -1) {
			lastMouseOverPos = glm::i32vec2(msg.x,msg.y);
			liveLog << "\nvalue " << msg.value;
			onHotTextMouseOver(msg.value);
		}
	}

	if (msg.Msg == uiMsgDelete && sender->id == popObjWinId) {
		static_cast<CGUIrichTextPanel*>(sender)->setStatus(CGUIrichTextPanel::readyToDelete);
	}

}

/** Respond to player mousing over hot text.*/
void CWorldUI::onHotTextMouseOver(int hotId) {
	TFnCall fnCall = pVM->getHotTextFnCall(hotId, currentVariant);
	currentMouseOverObj = fnCall.objId;
	for (auto& popWin : popupWindows) {
		if (fnCall.objId == popWin.objId && popWin.win->status == CGUIrichTextPanel::displaying) {
			popWin.lifeTime = 0;
			return;
		}
		//for now, we close any other pop up window
		//later this needs to be naunced: if a window is the sender of our mouseover (need a 2nd param)
		//we don't close it
		popWin.win->setStatus(CGUIrichTextPanel::readyToDelete);
	}

	pVM->callMember(fnCall.objId, fnCall.msgId, fnCall.params);
}

/** Position this popup window near to the mouse position when it was spawned, but adjusted
	to keep it inside the screen. */
void CWorldUI::positionPopupWindow(CGUIrichTextPanel* popupWin) {
	glm::i32vec2 mousePos = lastMouseOverPos;
	glm::i32vec2 newCornerPos = mousePos - glm::i32vec2(0, popupWin->getHeight());

	if (newCornerPos.y < 1)
		newCornerPos.y = mousePos.y + popupWin->getFont()->lineHeight;
	if (newCornerPos.x + popupWin->getWidth() > popupWin->parent->getWidth())
		newCornerPos.x = mousePos.x - popupWin->getWidth();
	popupWin->setLocalPos(newCornerPos.x, newCornerPos.y);

	popupWin->setStatus(CGUIrichTextPanel::displaying);
}

