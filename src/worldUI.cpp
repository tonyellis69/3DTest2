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
	pVM->execute(); //for any global variables or code
	playerId = pVM->getGlobalVar("playerObj").getObjId();

	createTextStyles();

	createTextWindow();
	createInventoryWindow();

	//set styles here
	mainTextPanel->addStyle(mainHeaderStyle);
	mainTextPanel->addStyle(mainBodyStyle);
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




/** Player has clicked hot text in the main windiw. */
void CWorldUI::mainWindowClick(int msgId, int objId, glm::i32vec2 mousePos) {
	//has the user clicked on hot text for player movement?
	int currentRoomId = pVM->callMember(playerId, "parent", {}).getObjId();
	if (objId == currentRoomId) {
		pVM->callMember(playerId, "moveTo", { msgId });
		return;
	}

	currentMousePos = mousePos;
	pVM->callMember(objId, "click");
}

/** Handle a click on an inventory window item with the given object id. */
void CWorldUI::inventoryClick(int msgId, int objId, const glm::i32vec2& mousePos) {
	currentMousePos = mousePos;
	pVM->callMember(objId, "click");
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
	pMenuWindow->setTextStyle(popBodyStyle);
	pMenuWindow->setHotTextColour(hottextColour);
	pMenuWindow->setHotTextHighlightColour(hottextSelectedColour);
	pMenuWindow->setResizeMode(resizeByWidthMode);
	pMenuWindow->id = popMenuId;
}

void CWorldUI::openObjWindow(int objId) {
	for (auto objWindow : objWindows) {
		if (objWindow.objId == objId)
			return;
	}

	CGUIrichTextPanel* pop = spawnPopText();
	pop->addStyle(popHeaderStyle);
	pop->addStyle(popBodyStyle);
	pop->resize(300, 200);
	pop->setHotTextColour(hottextColour);
	pop->setHotTextHighlightColour(hottextSelectedColour);
	pop->setResizeMode(resizeByRatioMode);
	pop->id = popObjWinId;
	objWindows.push_back({ pop,objId });
}


void CWorldUI::purge(int memberId, int objId) {
	mainTextPanel->purgeHotText(memberId, objId);
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
void CWorldUI::menuClick(const int msgId, int objId, glm::i32vec2& mousePos, CGUIrichTextPanel* popUp) {
	currentMousePos = mousePos;
	pVM->callMember(objId, msgId);
	popUp->destroy();
}

/** Respond to user clicking on an object window.*/
void CWorldUI::objWindowClick(const int msgId, int objId, glm::i32vec2& mousePos, CGUIrichTextPanel* popUp) {
	currentMousePos = mousePos;

	if (msgId == -1) { //clicked outside window, so we want to dismiss this window
		objWindows.pop_back();
		popUp->destroy();
		return;
	}

	pVM->callMember(objId, msgId);
}





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

void CWorldUI::vmMessage(int p1, int p2) {
	if (p1 == msgRoomChange)
		handleRoomChange(p2);
}

/** Create the main text window. */
void CWorldUI::createTextWindow() {
	mainTextPanel = new CGUIrichTextPanel(200, 50, 800, 700);
	mainTextPanel->setBackColour1(uiWhite);
	mainTextPanel->setBackColour2(uiWhite);
	
	mainTextPanel->borderOn(true);
	mainTextPanel->hFormat = hCentre;
	mainTextPanel->setInset(40);
	mainTextPanel->setFont(&pApp->popFont);
	mainTextPanel->setTextColour(UIwhite);
	mainTextPanel->setResizeMode(resizeByWidthMode);
	//popupTextID = popupPanel->getRichTextID();
	mainTextWindowID = mainTextPanel->getID();
	
	pApp->GUIroot.Add(mainTextPanel);
}

void CWorldUI::createInventoryWindow() {
/*	CGUIpanel* backPanel = new CGUIpanel(1100, 120, 180, 300);   //1100, 120, 180, 300
	UIcolour tint = { 0,0,0,0.3f };
	backPanel->setBackColour1(tint);
	backPanel->setBackColour2(tint);
	backPanel->borderOn(true);
	backPanel->anchorRight = 10;
	backPanel->hFormat = hRight;
	pApp->GUIroot.Add(backPanel);

	invWindow = new CGUIrichText(10, 10, 160, 180); //was 160 280
	invWindow->hFormat = hCentre;
	invWindow->borderOn(false);
	invWindow->setMultiLine(true);
	backPanel->Add(invWindow);
	invWindowID = invWindow->getID(); */




	invPanel = new CGUIrichTextPanel(1100, 120, 180, 300);
	UIcolour tint = { 0,0,0,0.3f };
	invPanel->setBackColour1(tint);
	invPanel->setBackColour2(tint);

	invPanel->borderOn(true);
	invPanel->anchorRight = 10;
	invPanel->hFormat = hRight;
	invPanel->setInset(10);
	invPanel->setFont(&pApp->popFont);
	invPanel->setTextColour(UIwhite);
	invPanel->setResizeMode(resizeByWidthMode);
	//popupTextID = popupPanel->getRichTextID();
	invPanelID = invPanel->getID();

	pApp->GUIroot.Add(invPanel);
}

CGUIrichTextPanel* CWorldUI::spawnPopText() {
	UIcolour tint = { 0,0,0,0.7f };
	CGUIrichTextPanel* popupPanel = new CGUIrichTextPanel(0, 0, 300, 300);
	popupPanel->setBackColour1(tint);
	popupPanel->setBackColour2(tint);
	popupPanel->borderOn(true);
	popupPanel->setFont(&pApp->popFont);
	popupPanel->setTextColour(UIwhite);
	popupPanel->setResizeMode(resizeByWidthMode);
	popupTextID = popupPanel->getRichTextID();
	popupPanelID = popupPanel->getID();
	popupPanel->setVisible(false);
	pApp->GUIroot.addModal(popupPanel);
	return popupPanel;
}

/** Create the various text styles the various text controls use. */
void CWorldUI::createTextStyles() {
	glm::vec4 darkGray(0.25, 0.25, 0.25, 1);

	//CFont* main = &pApp->renderer.fontManager.getFont("work16L");
//	CFont* header = &pApp->renderer.fontManager.getFont("work16");

	mainBodyStyle = { "mainBody","work16L",darkGray };
	mainHeaderStyle = { "mainHeader","work16",darkGray };
	invBodyStyle = { "invBody","work16L",darkGray };
	popBodyStyle = { "popBody", "work16L",darkGray };
	popHeaderStyle = { "popHeader","work16",darkGray };
}

