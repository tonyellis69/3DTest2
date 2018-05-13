#include "worldUI.h"

#include <ctype.h>

void CWorldUI::setVM(CTigVM * vm) {
	pVM = vm;
}

void CWorldUI::setTextWindow(CGUIrichText * txtWin) {
	pTextWindow = txtWin;
}

void CWorldUI::setInventoryWindow(CGUIrichText * invWin) {
	pInvWindow = invWin;
}

void CWorldUI::setCurrentWindow(CGUIrichText * pWin) {
	currentTextWindow = pWin;
}


void CWorldUI::setPopupTextWindow(CGUIrichTextPanel * pPopText) {
	popControl = pPopText;
}


void CWorldUI::init() {
	setCurrentWindow(pTextWindow);
	pVM->execute();
	currentRoom = pVM->getGlobalVar("startRoom"); //TO DO: scrap
	currentRoomNo = currentRoom.getObjId();
	playerId = pVM->getGlobalVar("playerObj").getObjId();
	findMoveToIds();
	findTreeIds();
	clickedHotText = 0;
}

void CWorldUI::findMoveToIds() {
	moveToIds[moveNorth] = pVM->getMemberId("northTo");
	moveToIds[moveNE] = pVM->getMemberId("neTo");
	moveToIds[moveEast] = pVM->getMemberId("eastTo");
	moveToIds[moveSE] = pVM->getMemberId("seTo");
	moveToIds[moveSouth] = pVM->getMemberId("northTo");
	moveToIds[moveSW] = pVM->getMemberId("swTo");
	moveToIds[moveWest] = pVM->getMemberId("westTo");
	moveToIds[moveNW] = pVM->getMemberId("nwTo");
	moveToIds[moveUp] = pVM->getMemberId("upTo");
	moveToIds[moveDown] = pVM->getMemberId("downTo");
	moveToIds[moveIn] = pVM->getMemberId("inTo");
	moveToIds[moveOut] = pVM->getMemberId("outTo");
}

void CWorldUI::findTreeIds() {
	parentId = pVM->getMemberId("parent");
	childId = pVM->getMemberId("child");
	siblingId = pVM->getMemberId("sibling");
}

/** Compiles the current room's description and sends it for display. */
void CWorldUI::roomDescription() {
	//setCurrentWindow(pTextWindow); //TO DO: scrap
	//CTigVar result = pVM->ObjMessage(currentRoom, "description");
	std::string result = pVM->ObjMessage(currentRoom, "description").getStringValue();
	result = markupHotText(result);
	pTextWindow->appendMarkedUpText(result);

	//List any contents
	refreshLocalList();
	refreshInvWindow();
	int item = child(currentRoomNo);
	if (!item)
		return;
	std::string itemsText("\n\nI could see");
	do {
		int localId = localHotList.addObject(item);
		itemsText += makeHotText(" a " + pVM->ObjMessage(item, "name").getStringValue(), localId);
		if (sibling(item)) {
			if (sibling(sibling(item)))
				itemsText +=  ",";
			else
				itemsText += " and";
		}
	} while (objectInLoop(currentRoomNo, item));
	itemsText += " here.";
	itemsText = markupHotText(itemsText);
	pTextWindow->appendMarkedUpText(itemsText);
}

/** Start a game session. */
void CWorldUI::start() {
	roomDescription();
}


/** Add this as a recognised hot text expression. */
void CWorldUI::addHotText(std::string & text, int id) {
	THotTextRec hotText;
	hotText.text = text;
	hotText.id = id;
	hotTextList.push_back(hotText);
}

/** Checking for hot text and style markups, turn the given text into one or more rich-text
	instructions sent to the text control. */
void CWorldUI::processText(string text) {
	text = markupHotText(text);
	currentTextWindow->appendMarkedUpText(text);
	//writeRichText(text, currentTextWindow);
}

/** If any registered hot text is found in the given text, mark it up for further processing.*/
std::string CWorldUI::markupHotText(std::string & text) {
	for (auto hotText : hotTextList) {
		size_t found = text.find(hotText.text);
		while (found != std::string::npos) {
			if ((found > 0 && !isalnum(text[found - 1])) &&
				(!isalnum(text[found + hotText.text.size()]))) {
				std::string tag = makeHotText(hotText.text, hotText.id);
				text.replace(found, hotText.text.size(), tag);
				found += tag.size();
			}
			found = text.find( hotText.text, found + hotText.text.size());
		}
	}
	return text;
}

/** Handle the player clicking on a piece of hot text. */
void CWorldUI::hotTextClick(int hotId, glm::i32vec2 mousePos) {
	clickedHotText = hotId;
	//is this a move command?
	for (int dir = 0; dir < 12; dir++) {
		if (hotId == moveToIds[dir]) {
			changeRoom(hotId);
			return;
		}
	}

	//is it a click on an item in the room description? check range
	if (hotId < memberIdStart) {
		int objId = localHotList.getObjectId(hotId);
		objectClick(objId,mousePos);
	}
	
}

/** Handle a click on an inventory window item with the given object id. */
void CWorldUI::inventoryClick(int hotId, const glm::i32vec2& mousePos) {
	int objId = localHotList.getObjectId(hotId);
	objectClick(objId, mousePos);
}

/** Change current room. */
void CWorldUI::changeRoom(int moveId) {
	CTigVar member = pVM->getMember(currentRoom, moveId);
	if (member.type == tigObj) {
		currentRoom = member;
		currentRoomNo = member.getObjId();
		pTextWindow->purgeHotText();
		pTextWindow->appendText("\n\n");
		roomDescription();
	}
}


/** Pick up this object. */
void CWorldUI::take(int objId) {
	pTextWindow->purgeHotText(clickedHotText);
	std::string takeText = "\n\nI picked up the ";
	takeText += makeHotText(pVM->ObjMessage(objId, "name").getStringValue(), localHotList.getLocalId(objId));
	takeText = markupHotText(takeText + ".");
	pTextWindow->appendMarkedUpText(takeText);
	move(objId, playerId);
	refreshInvWindow();
}

/** Drop this object. */
void CWorldUI::drop(int objId) {
	int localId = localHotList.getLocalId(objId);
	pTextWindow->purgeHotText(localId);
	move(objId, currentRoomNo);
	refreshInvWindow();
	std::string dropText = "\n\nI dropped the ";
	dropText += makeHotText(pVM->ObjMessage(objId, "name").getStringValue(), localId);
	dropText = markupHotText(dropText + ".");
	pTextWindow->appendMarkedUpText(dropText);
}

void CWorldUI::examine(int objId) {
	int localId = localHotList.getLocalId(objId);
	//pTextWindow->purgeHotText(localId);
	int currentRextent = popControl->drawBox.pos.x + popControl->getWidth();
	popControl->clear();
	popControl->resize(250, 200);
	popControl->setTextStyle(popHeaderStyle);
	//popControl->setTextColour(UIhiGrey);



	std::string examText  = cap(pVM->ObjMessage(objId, "name").getStringValue())
		+ "\n";
	examText = markupHotText(examText);
	popControl->appendMarkedUpText(examText);
//	popControl->setFont(popBodyFont);
//	popControl->setTextColour(UIwhite);
	popControl->setTextStyle(popBodyStyle);
	examText = pVM->ObjMessage(objId, "description").getStringValue();
	examText = markupHotText(examText);

	popControl->appendMarkedUpText(examText);
	popControl->appendText("\n");

	//add convenience options, ie, take/drop
	popChoices.clear();
	if (parent(objId) == currentRoomNo) {
		popChoices.push_back({ "Take", popTake });
	}
	if (parent(objId) == playerId) {
		popChoices.push_back({ "Drop", popDrop });
	}
	popChoices.push_back({ "Do nothing", popDoNothing });
	appendChoicesToPopup();

	popControl->resizeToFit();

	glm::i32vec2 cornerPos = popControl->drawBox.pos;
	if (cornerPos.x + popControl->getWidth() > popControl->parent->getWidth())
		cornerPos.x = currentRextent - popControl->getWidth();
	showPopupMenu(cornerPos);
}

/** Return the index of first child of the given parent, if any. */
int CWorldUI::child(int parent) {
	return pVM->getMemberValue(parent, childId);
}

/** Return index of sibling of given object, if any. */
int CWorldUI::sibling(int object) {
	return pVM->getMemberValue(object, siblingId);
}

/** Return index of parent of given object, if any. */
int CWorldUI::parent(int childNo) {
	return pVM->getMemberValue(childNo, parentId);
}

/** Return true while parent has a descendant and child = 0 or a decendant with a sibling .*/
bool CWorldUI::objectInLoop(int parent, int& childNo) {
	if (childNo == 0) 
		childNo = child(parent);
	else
		childNo = sibling(childNo);
	return (bool)childNo;
}

/** Make the given object a child of the destination object. */
void CWorldUI::move(int obj, int dest) {
	int parentObj = parent(obj);
	int childObj = child(parentObj); int siblingObj = 0;
	while (childObj != obj) {
		siblingObj = childObj;
		childObj = sibling(childObj);
	}
	if (siblingObj)
		pVM->objects[siblingObj].members[siblingId].setObjId(sibling(obj));
	else
		pVM->objects[parentObj].members[childId].setObjId(sibling(obj));

	int destChild = child(dest);
	if (destChild) {
		pVM->objects[obj].members[siblingId].setObjId(destChild);
	} else
		pVM->objects[obj].members[siblingId].setObjId(0);
	pVM->objects[dest].members[childId].setObjId(obj);
	pVM->objects[obj].members[parentId].setObjId(dest);
}

/** Ensure inventory window shows the latest player contents. */
void CWorldUI::refreshInvWindow() {
	pInvWindow->clear();
	pInvWindow->appendMarkedUpText("Inventory:\n");
	std::string invText;
	int item = child(playerId);
	if (item) {
		do {
			int localId = localHotList.getLocalId(item);
			invText += makeHotText("\nA " + pVM->ObjMessage(item, "name").getStringValue(), localId);
		} while (objectInLoop(playerId, item));
		invText = markupHotText(invText);
		pInvWindow->appendMarkedUpText(invText);
	}
}

/** Clear the local list and repopulate it with items the player is carrying. */
void CWorldUI::refreshLocalList() {
	localHotList.clear();
	int item = child(playerId);
	if (item)
		do {
			localHotList.addObject(item);
		} while (objectInLoop(playerId, item));
}

/** Handle a user-click on this object. */
void CWorldUI::objectClick(int objId, const glm::i32vec2& mousePos) {
	clickedObj = objId;
	popControl->clear();

	popChoices.clear();
	popControl->resize(200, 200);
	//acquire the different options available
	//is it on the ground? We can take it
	if (parent(objId) == currentRoomNo)
		popChoices.push_back({ "Take", popTake });
	if (parent(objId) == playerId)
		popChoices.push_back({ "Drop", popDrop });
	popChoices.push_back({ "Do nothing",popDoNothing });
	popChoices.insert(popChoices.end()-1,{ "Examine", popExamine });
	appendChoicesToPopup();

	popControl->resizeToFit();

	glm::i32vec2 cornerPos = mousePos + glm::i32vec2(0,pTextWindow->getFont()->lineHeight / 2);
	if (cornerPos.x + popControl->getWidth() > popControl->parent->getWidth())
		cornerPos.x = mousePos.x - popControl->getWidth();
	showPopupMenu(cornerPos);
}

void CWorldUI::appendChoicesToPopup() {
	string popStr; int choiceNo = 1;
	for (auto item : popChoices) {
		popStr += makeHotText(item.actionText, choiceNo);
		if (choiceNo < popChoices.size())
			popStr += "\n";
		choiceNo++;
	}
	markupHotText(popStr);
	popControl->appendMarkedUpText(popStr);
}

void CWorldUI::showPopupMenu(const glm::i32vec2& cornerPos) {
	popControl->setPos(cornerPos.x,cornerPos.y);
	popControl->setVisible(true);
	popControl->makeModal(popControl);
}

std::string CWorldUI::makeHotText(std::string text, int idNo) {
	std::string hotStr = "\\h{" + std::to_string(idNo) + "}";
	hotStr += text + "\\h";
	return hotStr;
}

/** Respond to the user selecting an item from the popup menu. */
void CWorldUI::popupSelection(int choice, glm::i32vec2& mousePos) {
	currentMousePos = mousePos;
	if (choice == -1)
		return;
	TPopAction action = popChoices[choice-1].action;
	switch (action) {
		case popTake: take(clickedObj); break;
		case popDrop: drop(clickedObj); break;
		case popExamine: examine(clickedObj); break;
	}


}

std::string CWorldUI::cap(std::string text) {
	text[0] = toupper(text[0]);
	return text;
}


void CWorldUI::setMainBodyStyle( CFont& font, const glm::vec4& colour) {
	mainBodyStyle = { &font,colour };
	pTextWindow->setTextStyle(mainBodyStyle);
}

void CWorldUI::setInvBodyStyle(CFont & font, const glm::vec4 & colour) {
	invBodyStyle = { &font,colour };
	pInvWindow->setTextStyle(invBodyStyle);
}

void CWorldUI::setPopBodyStyle(CFont & font, const glm::vec4 & colour) {
	popBodyStyle = { &font,colour };
	popControl->setTextStyle(popBodyStyle);
}

void CWorldUI::setPopHeaderStyle(CFont & font, const glm::vec4 & colour) {
	popHeaderStyle = { &font,colour };
	popControl->setTextStyle(popHeaderStyle);
}

void CWorldUI::setHottextColour(const glm::vec4 & colour) {
	hottextColour = colour;
	pTextWindow->setHotTextColour(hottextColour);
	pInvWindow->setHotTextColour(hottextColour);
	popControl->setHotTextColour(hottextColour);
}

void CWorldUI::setHottextSelectColour(const glm::vec4 & colour) {
	hottextSelectedColour = colour;
	pTextWindow->setHotTextHighlightColour(hottextSelectedColour);
	pInvWindow->setHotTextHighlightColour(hottextSelectedColour);
	popControl->setHotTextHighlightColour(hottextSelectedColour);
}

