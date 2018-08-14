#include "worldUI.h"

#include <ctype.h>

#include "3DtestApp.h"

void CWorldUI::setVM(CTigVM * vm) {
	pVM = vm;
}

void CWorldUI::setTextWindow(CGUIrichText * txtWin) {
	pTextWindow = txtWin;
}

void CWorldUI::setInventoryWindow(CGUIrichText * invWin) {
	pInvWindow = invWin;
}

void CWorldUI::setGameApp(C3DtestApp * app) {
	pApp = app;
}

void CWorldUI::init() {
	pVM->execute(); //for any global variables or code
	playerId = pVM->getGlobalVar("playerObj").getObjId();
}


/** Start a game session. */
void CWorldUI::start() {
	pVM->callMember(zeroObject, "init"); 	//execute Tig initialisation code
	int currentRoomId = pVM->callMember(playerId, "parent").getObjId();
	pVM->callMember(currentRoomId, "look");
}


void CWorldUI::appendText(std::string & text, int window) {
	if (window == mainWin)
		pTextWindow->appendMarkedUpText(text);
	else if (window == invWin)
		pInvWindow->appendMarkedUpText(text);
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
	pMenuWindow = pApp->spawnPopText();
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

	CGUIrichTextPanel* pop = pApp->spawnPopText();
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
	pTextWindow->purgeHotText(memberId, objId);
}

void CWorldUI::clearWindow(int window) {
	if (window == mainWin)
		pTextWindow->clear();
	else if (window == invWin)
		pInvWindow->clear();
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
	glm::i32vec2 newCornerPos = cornerPos + glm::i32vec2(0, pTextWindow->getFont()->lineHeight  );
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



void CWorldUI::setMainBodyStyle( CFont& font, const glm::vec4& colour) {
	mainBodyStyle = { "mainBody",&font,colour };
	pTextWindow->setTextStyle(mainBodyStyle);
}

void CWorldUI::setInvBodyStyle(CFont & font, const glm::vec4 & colour) {
	invBodyStyle = {"invBody", &font,colour };
	pInvWindow->setTextStyle(invBodyStyle);
}

void CWorldUI::setPopBodyStyle(CFont & font, const glm::vec4 & colour) {
	popBodyStyle = {"popBody", &font,colour };

}

void CWorldUI::setPopHeaderStyle(CFont & font, const glm::vec4 & colour) {
	popHeaderStyle = {"popHeader", &font,colour };

}

void CWorldUI::setHottextColour(const glm::vec4 & colour) {
	hottextColour = colour;
	pTextWindow->setHotTextColour(hottextColour);
	pInvWindow->setHotTextColour(hottextColour);
}

void CWorldUI::setHottextSelectColour(const glm::vec4 & colour) {
	hottextSelectedColour = colour;
	pTextWindow->setHotTextHighlightColour(hottextSelectedColour);
	pInvWindow->setHotTextHighlightColour(hottextSelectedColour);;
}

void CWorldUI::vmMessage(int p1, int p2) {
	if (p1 == msgRoomChange)
		handleRoomChange(p2);
}

