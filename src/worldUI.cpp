#include "worldUI.h"
#include "worldUI.h"
#include "worldUI.h"
#include "worldUI.h"
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
	mouseoverId = pVM->getMemberId("mouseOver");
	examId = pVM->getMemberId("examine");
	attemptMoveId = pVM->getMemberId("attemptMove");
	showPlayerOpsId = pVM->getMemberId("showPlayerOptions");
	gameStateId = pVM->getGlobalVar("gameStateObj").getObjId();
	tidyModeMask = pVM->getFlagBitmask("tidyMode");
	
	//got here
	createMainWindow();

	createInventoryWindow();

	pMenuWindow = NULL; //TO DO: should probably create and make invisible to be consistent
	currentVariant = 0;
	
	createDistributor();
	createHealthPanel();

}


/** Start a game session. */
void CWorldUI::start() {
	pVM->callMember(zeroObject, "init"); 	//execute Tig initialisation code
	updateDistributorGUI();
	updateHealthGUI();
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

	if (fnCall.msgId == mouseoverId) //plauyer clicked on hot text that supplies pop-ups only
		return;

	
	mainTextPanel->unhotDuplicates();

	//do we want to refresh the room description?
	if (pVM->hasFlag(gameStateId, tidyModeMask)) {
		queueMsg(TvmAppMsg{ appClearToBookmark });

		queueMsg(TvmAppMsg{ appSetLineFadein,"","",1 });
		pVM->callMember(NULL, "globalLook");

		queueMsg(TvmAppMsg{ appFinishDisplay});
		queueMsg(TvmAppMsg{ appSetLineFadein,"","",0 });

		queueMsg(TvmAppMsg{ appWriteText,"","\n" + choiceTxt });
	}


	queueMsg(TvmAppMsg{ appSolidifyTmpText }); 
	//this prevents the choice text being cleared when the uses mouses off their choice

	//TO DO: push a copy of fnCall to an array to record player activity for playback

	pVM->callMember(fnCall.objId, fnCall.msgId,fnCall.params);

	//instantaneous actions don't end the player's turn
	if (fnCall.msgId == showPlayerOpsId )
		return; 

	pVM->callMember(NULL, "gameTurn");

	updateDistributorGUI();
}



/** Register change in current room. */
void CWorldUI::handleRoomChange(int roomId) {
	//TO DO: ever doing anything here?
}

/** Respond to a VM message updating the available power in the accumulator. */
void CWorldUI::onVMaccumulatorUpdate(int powerUpdate) {
	//liveLog << "\naccumulator update of " << powerUpdate;
	distributor->setAvailablePower(powerUpdate);
}

/** Respond to a VM message updating the available power in the current power cell. */
void CWorldUI::onVMpowerCellUpdate(int powerUpdate) {
	healthPanel->setPower(powerUpdate);
}

void CWorldUI::onVMhpChange(int hpUpdate) {
	healthPanel->setHP(hpUpdate);
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
	//pMenuWindow->makeModal();
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
	pop->setObjId(objId);
	popupWindows.push_back({ pop,objId,0 });

}


void CWorldUI::purgeMainPanel(unsigned int id) {
	return;
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

/** Respond to user clicking on a popup window.*/
void CWorldUI::popupWindowClick(unsigned int hotId, glm::i32vec2 mousePos, CGUIrichTextPanel* popUp) {
	lastMouseOverPos = mousePos;
	playerTurn(hotId);
	//closeObjWindow(popUp);

	deletePopupWindow(popUp->getObjId());

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

/** Handle a message from the VM. */
void CWorldUI::vmMessage(const std::string& p1, int p2) {
	if (p1 == "roomChange")
		handleRoomChange(p2);
	if (p1 == "accumulatorUpdate")
		onVMaccumulatorUpdate(p2);
	if (p1 == "powerCellUpdate")
		onVMpowerCellUpdate(p2);
	if (p1 == "HPchange")
		onVMhpChange(p2);

}

/** add the given message to the queue for eventual processing. */
void CWorldUI::queueMsg(TvmAppMsg& msg) {
	messages.push(msg);
}

/** Create the main text window. */
void CWorldUI::createMainWindow() {
	mainTextPanel = new CGUIrichTextPanel(200, 50, 800, 700);
	//mainTextPanel->setBackColour1(style::uiWhite);
	//mainTextPanel->setBackColour2(style::uiWhite);
	
	mainTextPanel->setBorderOn(true);
	mainTextPanel->hFormat = hCentre;
	mainTextPanel->setRichtextInset(10);
	//mainTextPanel->setTextColour(style::uiWhite);
	mainTextPanel->setResizeMode(resizeByWidthMode);
	mainTextWindowID = mainTextPanel->getUniqueID();
	//mainTextPanel->setTextStyles(&normalTheme.styles);
	mainTextPanel->setTextTheme("gameNormal");
	//mainTextPanel->setTextStyle("mainBody");
	///////////////mainTextPanel->setDefaultTextStyle("mainBody");
	pApp->GUIroot.add(mainTextPanel);
	mainTextPanel->richText->transcriptLog = &transcript;
	mainTextPanel->deliveryMode = byCharacter;// byClause;
	mainTextPanel->setLineFadeIn(false);
	mainTextPanel->setGUIcallback(this);
}

void CWorldUI::createInventoryWindow() {
	invPanel = new CGUIrichTextPanel(1100, 50, 180, 390);
	invPanel->setBackColour1(white);
	invPanel->setBackColour2(white);
	
	invPanel->setBorderOn(true);
	invPanel->anchorRight = 10;
	invPanel->hFormat = hRight;
	invPanel->setRichtextInset(10);
	invPanel->setTextColour(UIwhite);
	
	invPanel->setResizeMode(resizeByWidthMode);
	//got here
	invPanelID = invPanel->getUniqueID();
	//invPanel->setTextStyles(&smallNormalTheme.styles);
	invPanel->setTextTheme("smallNormal");
	//////////////////////invPanel->setDefaultTextStyle("small");
	invPanel->setTextStyle("smallHeader");
	//got here
	invPanel->displayText("Inventory:");
	
	pApp->GUIroot.add(invPanel);
	invPanel->setGUIcallback(this);
	//didn't get here
}


CGUIrichTextPanel* CWorldUI::spawnPopText(bool modal) {
	CGUIrichTextPanel* popupPanel = new CGUIrichTextPanel(0, 0, 300, 300);
	popupPanel->setBackColour1(white);
	popupPanel->setBackColour2(white);
	popupPanel->setBorderOn(true);
//	popupPanel->setFont(&pApp->popFont);
	popupPanel->setTextColour(UIwhite);
	popupPanel->setResizeMode(resizeByWidthMode);
	popupPanel->setShortestSpaceBreak(50);
	popupTextID = popupPanel->getRichTextID();
	popupPanelID = popupPanel->getUniqueID();
	popupPanel->setGUIcallback(this);
	popupPanel->setVisible(false);
	//popupPanel->setTextStyles(&smallNormalTheme.styles);
	mainTextPanel->setTextTheme("gameNormal");
	popupPanel->setTextTheme("smallNormal");

	//if (modal)
	//	pApp->GUIroot.addModal(popupPanel);
	//else
		pApp->GUIroot.add(popupPanel);
	return popupPanel;
}


void CWorldUI::createDistributor() {
	distributor = new CGUIdistributor(1100, 450, 180, 300);
	distributor->setBackColour1(white);
	distributor->setBackColour2(white);
	distributor->setBorderOn(true);
	distributorID = distributor->getUniqueID();
	//TO DO: ideally id should be tied to obj id of the vm distributor object

	distributor->setGUIcallback(this);
	distributor->setBorderOn(true);
	distributor->anchorRight = 10;
	distributor->hFormat = hRight;
	pApp->GUIroot.add(distributor);
}

void CWorldUI::createHealthPanel() {
	int yStart = distributor->getLocalPos().y + distributor->getHeight() + 10;

	healthPanel = new CGUIhealthPanel(1100, yStart, 180, 300);
	healthPanel->setBackColour1(white);
	healthPanel->setBackColour2(white);
	//TO DO: shouldn't need this with stylesheets! fix!
	healthPanel->setBorderOn(true);
	healthPanelID = healthPanel->getUniqueID();

	healthPanel->setGUIcallback(this);
	healthPanel->setBorderOn(true);
	healthPanel->anchorRight = 10;
	healthPanel->hFormat = hRight;
	pApp->GUIroot.add(healthPanel);


}

/** Create the various text styles the various text controls use. */
void CWorldUI::createTextStyles() {
	darkGray = glm::vec4(0.25, 0.25, 0.25, 1);
	choiceColour = glm::vec4(0.5, 0.5, 0.5, 1);
	white = glm::vec4(1);
	hottextColour =  glm::vec4(0.28, 0.28, 0.47, 1);
	hottextSelectedColour =  glm::vec4(1, 0.547, 0.0, 1);

/*
	pApp->gameStyleSheet.addToTheme("gameNormal", { "mainBody", "main", darkGray });
	pApp->gameStyleSheet.addToTheme("gameNormal", { "mainHeader","mainHeader",darkGray });
	pApp->gameStyleSheet.addToTheme("gameNormal", { "hot", "main", hottextColour });
	pApp->gameStyleSheet.addToTheme("gameNormal", { "hotSelected", "main", hottextSelectedColour });
	pApp->gameStyleSheet.addToTheme("gameNormal", { "choice", "main", choiceColour });


	pApp->gameStyleSheet.addToTheme("smallNormal", { "small","small",darkGray });
	pApp->gameStyleSheet.addToTheme("smallNormal", { "smallHeader","smallHeader",darkGray });
	pApp->gameStyleSheet.addToTheme("smallNormal", { "hot", "small", hottextColour });
	pApp->gameStyleSheet.addToTheme("smallNormal", { "hotSelected", "small", hottextSelectedColour });
	*/
}

void CWorldUI::hide(bool onOff) {
	invPanel->setVisible(!onOff);
	mainTextPanel->setVisible(!onOff);
	distributor->setVisible(!onOff);
	healthPanel->setVisible(!onOff);
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
bool CWorldUI::tempText(bool onOff, int winId) {
	return	mainTextPanel->setTempText(onOff);
}

void CWorldUI::update(float dT) {
	processMessageQueue();
	
	//TO DO: the impact of these two updates is *HUGE*. something definitely wrong
	mainTextPanel->update(dT);
	invPanel->update(dT);
	if (pMenuWindow)
		pMenuWindow->update(dT);

	for (auto& popupWindow : popupWindows) {
		popupWindow.win->update(dT); //CHECK: may want to do this only when displaying. 

		if (popupWindow.win->status == CGUIrichTextPanel::displaying) {
			popupWindow.lifeTime += dT;

			if (popupWindow.lifeTime > 0.15f && popupWindow.win->noMouse() &&
				popupWindow.objId != pVM->getHotTextFnCallObj(currentHotId) ) {
				deletePopupWindow(popupWindow.objId);
			}
			continue;
		}

		if (popupWindow.win->status == CGUIrichTextPanel::readyToDelete) {
			deletePopupWindow(popupWindow.objId);
			continue;
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

/** The player has moused off or onto hot text. */
void CWorldUI::onHotTextChange( int newHotId) {
	currentVariant = 0;
	currentHotId = newHotId;

	//if user has moused off of hot text
	if (currentHotId == noHotText) {
		queueMsg(TvmAppMsg{ appCollapseTmpTxt });
		return;
	}

	TFnCall fnCall = pVM->getHotTextFnCall(newHotId, currentVariant);
	//does this hot text fn call require a popup window?
	if (fnCall.msgId == mouseoverId) {
		popupRequest(fnCall);
		return;
	}

	//if not, it's a player choice
	queueMsg(TvmAppMsg{ appCollapseTmpTxt });
	handleChoiceText(currentHotId);
}

void CWorldUI::handleChoiceText(int hotId) {
	//string choiceTxt;
	TFnCall fnCall = pVM->getHotTextFnCall(hotId, currentVariant);
	if (!fnCall.params.empty()) {
		CTigVar finalParam = fnCall.params.back();
		if (finalParam.type == tigString) {
			choiceTxt = finalParam.getStringValue();
			for (unsigned int x = 0; x < choiceTxt.size(); x++) { //catch any hot text and suspend it
				if (choiceTxt[x] == '\\' && choiceTxt[x + 1] == 'h')
					choiceTxt[x + 1] = 'S';
			}
			//if (pVM->hasFlag(gameStateId, tidyModeMask)) {
				//queueMsg(TvmAppMsg{ appSetStyle,"markOn" });;
			//}
		//	queueMsg(TvmAppMsg{ appDisplayNarrativeChoice,choiceTxt });
			displayNarrativeChoice(choiceTxt);
		}
	}
	
}

/** Display the choice text associated with this hotId. */
bool CWorldUI::displayNarrativeChoice(std::string& choiceText) {
	/*if (mainTextPanel->busy()) {
		liveLog << "\nToo busy for choice text!";
		return false;
	}*/

//	mainTextPanel->setTempText(true);
	queueMsg(TvmAppMsg{ appTempTxt,"","",1,(int)mainTextWindowID });
	//mainTextPanel->setTextStyle("fadeOn");
	queueMsg(TvmAppMsg{ appSetStyle,"","fadeOn" });;

	mainTextPanel->deliveryMode = noDelivery;

	//mainTextPanel->setTextStyle("choice");
	queueMsg(TvmAppMsg{ appSetStyle,"","choice" });

	//mainTextPanel->displayText("\n\n" + choiceText);
	queueMsg(TvmAppMsg{ appWriteText,"","\n\n" + choiceText });

	mainTextPanel->deliveryMode = byCharacter;//byClause;
	
	//mainTextPanel->setTextStyle("fadeOff");
	queueMsg(TvmAppMsg{ appSetStyle,"","fadeOff" });

	//mainTextPanel->setTempText(false);
	queueMsg(TvmAppMsg{ appTempTxt,"","",0,(int)mainTextWindowID });

	//mainTextPanel->setTextStyle("mainBody");
	queueMsg(TvmAppMsg{ appSetStyle,"","mainBody" });

	return true;
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
	//mainTextPanel->collapseTempText();
	//displayNarrativeChoice(hotId);
	queueMsg(TvmAppMsg{ appCollapseTmpTxt });
	handleChoiceText(currentHotId);
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
//NB The point of queuing was supposed to be for the benefit of the VM, to ensure it wasn't sending
//instructions out into the world that couldn't yet be handled. If the queue is told an instruction 
//couldn't be received it keeps sending it, while safely queuing any later VM instructions behind it.
//But what about GUI messages such as the user mousing over or clicking on hot text? If they happen 
//when the system is busy they should just be thrown away, I don't want them to pile up. By themselves
//they don't require any third party to be ready for them. However, any instructions they give 
//eg to rich text panels probably should be queued.
//Try this out with mouseover, then clickon.
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
			case appMsg: vmMessage(msg.msgString, msg.integer2); break;
			case appTempTxt: result = tempText((bool)msg.integer, msg.integer2); break;
			case appPause: pause(true); break;
			case appUnpause: pause(false); break;
			case appClearMarked: clearMarkedText(msg.integer); break;
			//case appHotTxtChange: onHotTextChange(msg.integer); break;
			case appSolidifyTmpText: result = mainTextPanel->solidifyTempText(); break;
			case appCollapseTmpTxt: result = mainTextPanel->collapseTempText(); break;
			case appDisplayNarrativeChoice: result = displayNarrativeChoice(msg.text); break;
			case appSetStyle: result = mainTextPanel->setTextStyle(msg.text); break;
			case appClearToBookmark: result = mainTextPanel->clearToBookMark(); break;
			case appSetLineFadein: result = mainTextPanel->setLineFadeIn((bool)msg.integer); break;
			case appFinishDisplay: result = mainTextPanel->isDisplayFinished(); break;

		}

		if (result)
			messages.pop();
		else
			break;
	}
}

/** First calling point for callbacks from GUI controls such as the rich text panels. */
void CWorldUI::GUIcallback(CGUIbase* sender, CMessage& msg) {
	if (msg.Msg == uiMsgHotTextChange) {
		lastMouseOverPos = glm::i32vec2(msg.x,msg.y);
		//queueMsg(TvmAppMsg{ appHotTxtChange,"",msg.value,0,glm::i32vec2(msg.x,msg.y) });
		onHotTextChange(msg.value);
	}

	if (msg.Msg == uiMsgDelete && sender->id == popObjWinId) {
		static_cast<CGUIrichTextPanel*>(sender)->setStatus(CGUIrichTextPanel::readyToDelete);
	}

	if (msg.Msg == uiMsgHotTextClick) {
		if (sender->getUniqueID() == mainTextWindowID)
			mainWindowClick(msg.value, glm::i32vec2(msg.x, msg.y));
		else if (sender->id == popObjWinId)
			//objWindowClick(msg.value, glm::i32vec2(msg.x,msg.y), (CGUIrichTextPanel*)sender);
			popupWindowClick(msg.value, glm::i32vec2(msg.x, msg.y), (CGUIrichTextPanel*)sender);
		//TO DO clone to popupWindowClick 
		else if (sender->id == popMenuId)
			menuClick(msg.value, glm::i32vec2(msg.x, msg.y), (CGUIrichTextPanel*)sender);
	}


	if (msg.Msg == uiMsgRMouseUp && sender->getUniqueID() == mainTextWindowID) {
		mainWindowRightClick(glm::i32vec2(msg.x, msg.y));
	}

	if (msg.Msg == uiMsgUpdate && sender->getUniqueID() == distributorID) {
		setVMdistributor(msg.value, msg.value2);


	}
}

/** Respond to player mousing over popup hot text.*/
void CWorldUI::popupRequest(TFnCall& fnCall) {
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


void CWorldUI::flushMessageQueue() {
	//while (!messages.empty()) {
	//	processMessageQueue();
	//}
}

/** Set the values of the player's in-game distributor. */
void CWorldUI::setVMdistributor(int offence, int defence) {
	std::initializer_list<CTigVar> params = { offence,defence };
	pVM->callMember(0, "setDistributor", params);

}

/** Update the distributor display with the current available power. */
void CWorldUI::updateDistributorGUI() {
	CTigVar availPower = pVM->callMember(0, "getDistributorPower");
	distributor->setAvailablePower(availPower.getIntValue());
}

/** Update the distributor display with the current available power. */
void CWorldUI::updateHealthGUI() {
	CTigVar remainPower = pVM->callMember(0, "getRemainingPower");
	healthPanel->setPower(remainPower.getIntValue());

	CTigVar hp = pVM->callMember(0, "getPlayerHP");
	healthPanel->setHP(hp.getIntValue());
}


