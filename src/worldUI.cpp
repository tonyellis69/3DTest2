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



	createTextWindow();
	createInventoryWindow();
}


/** Start a game session. */
void CWorldUI::start() {
	pVM->callMember(zeroObject, "init"); 	//execute Tig initialisation code
	int currentRoomId = pVM->callMember(playerId, "parent").getObjId();
	pVM->callMember(currentRoomId, "look");
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
	THotTextFnCall fnCall = pVM->getHotTextFnCall(hotId);

	//has the user clicked on hot text for player movement?
	int currentRoomId = pVM->callMember(playerId, "parent", {}).getObjId();
	if (fnCall.objId == currentRoomId) {
		pVM->callMember(playerId, "moveTo", { fnCall.msgId });
		return;
	}
	//TO DO: replace above bodge with a clean call to player.moveTo(direction)
	//this can be the first test of parameters in hot func calls

	currentMousePos = mousePos;
	pVM->callMember(fnCall.objId, fnCall.msgId);
}

/** Handle a click on an inventory window item. */
void  CWorldUI::inventoryClick(unsigned int hotId, glm::i32vec2 mousePos) {
	THotTextFnCall fnCall = pVM->getHotTextFnCall(hotId);
	currentMousePos = mousePos;
	pVM->callMember(fnCall.objId, fnCall.msgId);
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
//	pMenuWindow->setHotTextColour(hottextColour);
//	pMenuWindow->setHotTextHighlightColour(hottextSelectedColour);
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
	//pop->setHotTextColour(hottextColour);
	//pop->setHotTextHighlightColour(hottextSelectedColour);
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

	popControl->setPos(newCornerPos.x, newCornerPos.y);
	popControl->setVisible(true);
}

/** Respond to the user selecting an item from the popup menu. */
void CWorldUI::menuClick(unsigned int hotId, glm::i32vec2& mousePos, CGUIrichTextPanel* popUp) {
	currentMousePos = mousePos;
	THotTextFnCall fnCall = pVM->getHotTextFnCall(hotId);
	pVM->callMember(fnCall.objId, fnCall.msgId);
	delete popUp;
}

/** Respond to user clicking on an object window.*/
void CWorldUI::objWindowClick(unsigned int hotId, glm::i32vec2 mousePos, CGUIrichTextPanel * popUp) {
	THotTextFnCall fnCall = pVM->getHotTextFnCall(hotId);
	currentMousePos = mousePos;
	pVM->callMember(fnCall.objId, fnCall.msgId);
}

void CWorldUI::closeObjWindow(CGUIrichTextPanel * popUp) {
	objWindows.pop_back();
	delete popUp;
	return;
}


/*
void CWorldUI::setHottextColour(const glm::vec4 & colour) {
	hottextColour = colour;
	mainTextPanel->setHotTextColour(hottextColour);
	invPanel->setHotTextColour(hottextColour);
}

void CWorldUI::setHottextSelectColour(const glm::vec4 & colour) {
	hottextSelectedColour = colour;
	mainTextPanel->setHotTextHighlightColour(hottextSelectedColour);
	invPanel->setHotTextHighlightColour(hottextSelectedColour);;
}
*/

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
	mainTextPanel->setInset(40);
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
	//invPanel->setFont(&pApp->popFont);
	invPanel->setTextColour(UIwhite);
	invPanel->setResizeMode(resizeByWidthMode);
	//popupTextID = popupPanel->getRichTextID();
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

