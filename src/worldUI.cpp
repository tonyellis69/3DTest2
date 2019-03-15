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
	createTextStyles();

	pVM->execute(); //for any global variables or code
	playerId = pVM->getGlobalVar("playerObj").getObjId();
	clickId = pVM->getMemberId("click");
	examId = pVM->getMemberId("examine");

	createTextWindow();
	createInventoryWindow();
}


/** Start a game session. */
void CWorldUI::start() {
	pVM->callMember(zeroObject, "init"); 	//execute Tig initialisation code
//	int currentRoomId = pVM->callMember(playerId, "parent").getObjId();
//	pVM->callMember(currentRoomId, "look");
}


void CWorldUI::appendText(std::string & text, int window) {
	if (window == mainWin)
		mainTextPanel->appendMarkedUpText(text);
	else if (window == invWin)
		invPanel->appendMarkedUpText(text);
	else if (window == menuWin) {
		pMenuWindow->appendMarkedUpText(text);
		if (!pMenuWindow->visible)
			showPopupMenu(pMenuWindow, currentMousePos);
		pMenuWindow->resizeToFit();
	}
	else { //obj window
		for (auto objWin : objWindows) {
			if (objWin.objId == window) {
				objWin.win->appendMarkedUpText(text);
				if (!objWin.win->visible)
					showPopupMenu(objWin.win, lastMenuCorner);
				objWin.win->resizeToFit();
				return;
			}
		}
	}
}


/**	Player has clicked on the identified hot text in the main window, so
	execute the function call stored for it. */
void CWorldUI::mainWindowClick(unsigned int hotId, glm::i32vec2 mousePos) {
	currentMousePos = mousePos;
	playerTurn(hotId);
}

/** Handle a click on an inventory window item. */
void  CWorldUI::inventoryClick(unsigned int hotId, glm::i32vec2 mousePos) {
	currentMousePos = mousePos;
	playerTurn(hotId);
}

/**	Carry out one complete turn of the game. This consists of anything that happens
	immediately before the player's action, the player's action itself (and any
	reactions), and then any game activity. */
void CWorldUI::playerTurn(unsigned int actionHotId) {
	THotTextFnCall fnCall = pVM->getHotTextFnCall(actionHotId);
	pVM->callMember(fnCall.objId, fnCall.msgId,fnCall.params);

	//instantaneous actions don't end the player's turn
	if (fnCall.msgId == clickId || fnCall.msgId == examId)
		return; 

	pVM->callMember(NULL, "gameTurn");
}

/** Register change in current room. */
void CWorldUI::handleRoomChange(int roomId) {
	//TO DO: ever doing anything here?
}

void CWorldUI::openWindow(int winId) {
	if (winId == menuWin)
		openMenuWindow(winId);
	else
		openObjWindow(winId);
}

void CWorldUI::openMenuWindow(int winId) {
	pMenuWindow = spawnPopText();
	pMenuWindow->setTextStyle("small");
	pMenuWindow->setResizeMode(resizeByWidthMode);
	pMenuWindow->id = popMenuId;
}

void CWorldUI::openObjWindow(int objId) {
	for (auto objWindow : objWindows) {
		if (objWindow.objId == objId)
			return;
	}

	CGUIrichTextPanel* pop = spawnPopText();
	pop->resize(300, 200);
	pop->setResizeMode(resizeByRatioMode);
	pop->id = popObjWinId;
	objWindows.push_back({ pop,objId });
}


void CWorldUI::purge(unsigned int id) {
	vector<unsigned int> purgedIds = mainTextPanel->purgeHotText(id);
	//whatever hot text was removed, remove also from the list of hot text function calls.
	for (auto purgedId : purgedIds)
		pVM->removeHotTextFnCall(purgedId);
}

void CWorldUI::clearWindow(int window) {
	if (window == mainWin)
		mainTextPanel->clear();
	else if (window == invWin)
		invPanel->clear();
	else {
		for (auto objWin : objWindows)
			if (objWin.objId == window) {
				objWin.win->clear();
				return;
			}
	}
}


/** Display the popup menu at the cursor position, adjusted for its dimensions and the screen edge. */
void CWorldUI::showPopupMenu(CGUIrichTextPanel* popControl, const glm::i32vec2& cornerPos) {
	lastMenuCorner = cornerPos;
	popControl->resizeToFit();
	glm::i32vec2 newCornerPos = cornerPos + glm::i32vec2(0, mainTextPanel->getFont()->lineHeight  );
	int margin = 30;
	if (newCornerPos.x + popControl->getWidth() + margin > popControl->parent->getWidth())
		newCornerPos.x -= (newCornerPos.x + popControl->getWidth() + margin) - popControl->parent->getWidth();

	if (newCornerPos.y + popControl->getHeight() + margin > popControl->parent->getHeight())
		newCornerPos.y -= (newCornerPos.y + popControl->getHeight() + margin) - popControl->parent->getHeight();

	popControl->setLocalPos(newCornerPos.x, newCornerPos.y);
	popControl->setVisible(true);
}

/** Respond to the user selecting an item from the popup menu. */
void CWorldUI::menuClick(unsigned int hotId, glm::i32vec2& mousePos, CGUIrichTextPanel* popUp) {
	currentMousePos = mousePos;
	delete popUp;
	playerTurn(hotId);
}

/** Respond to user clicking on an object window.*/
void CWorldUI::objWindowClick(unsigned int hotId, glm::i32vec2 mousePos, CGUIrichTextPanel * popUp) {
	currentMousePos = mousePos;
	playerTurn(hotId);
}

void CWorldUI::closeObjWindow(CGUIrichTextPanel * popUp) {
	objWindows.pop_back();
	delete popUp;
	return;
}


void CWorldUI::vmMessage(int p1, int p2) {
	if (p1 == msgRoomChange)
		handleRoomChange(p2);
}

/** Create the main text window. */
void CWorldUI::createTextWindow() {
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
	pApp->GUIroot.Add(mainTextPanel);
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
	invPanel->appendText("Inventory:");
	pApp->GUIroot.Add(invPanel);
}

CGUIrichTextPanel* CWorldUI::spawnPopText() {
	CGUIrichTextPanel* popupPanel = new CGUIrichTextPanel(0, 0, 300, 300);
	popupPanel->setBackColour1(white);
	popupPanel->setBackColour2(white);
	popupPanel->setBorderOn(true);
	popupPanel->setFont(&pApp->popFont);
	popupPanel->setTextColour(UIwhite);
	popupPanel->setResizeMode(resizeByWidthMode);
	popupTextID = popupPanel->getRichTextID();
	popupPanelID = popupPanel->getID();
	popupPanel->setVisible(false);
	popupPanel->setTextStyles(&smallNormalTheme.styles);
	pApp->GUIroot.addModal(popupPanel);
	return popupPanel;
}

/** Create the various text styles the various text controls use. */
void CWorldUI::createTextStyles() {
	darkGray = glm::vec4(0.25, 0.25, 0.25, 1);
	white = glm::vec4(1);
	hottextColour =  glm::vec4(0.28, 0.28, 0.47, 1);
	hottextSelectedColour =  glm::vec4(1, 0.547, 0.0, 1);

	normalTheme.styles.push_back({ "mainBody", "main", darkGray });
	normalTheme.styles.push_back({ "mainHeader","mainHeader",darkGray });
	normalTheme.styles.push_back({ "hot", "main", hottextColour });
	normalTheme.styles.push_back({ "hotSelected", "main", hottextSelectedColour });

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

