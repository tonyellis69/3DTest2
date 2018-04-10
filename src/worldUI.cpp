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

void CWorldUI::setPopupWindow(CGUIpopMenu * popMenu) {
	pPopMenu = popMenu;
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
	setCurrentWindow(pTextWindow);
	CTigVar result = pVM->ObjMessage(currentRoom, "description");
	processText(result.getStringValue());

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
	processText(itemsText + " here.");
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
	markupHotText(text);
	writeRichText(text, currentTextWindow);
}



void CWorldUI::writeRichText(string text, CGUIrichText* pWin) {
	bool bold = false; bool hot = false;
	enum TStyleChange { styleNone, styleBold, styleHot };

	std::string writeTxt = text;
	std::string remainingTxt = text;
	TStyleChange styleChange;
	while (remainingTxt.size()) {
		styleChange = styleNone;
		int cut = 0; int tagId = 0;
		size_t found = remainingTxt.find('\\');
		if (found != std::string::npos) {
			cut = 1;
			if (remainingTxt[found + 1] == 'b') {
				styleChange = styleBold;
				bold = !bold;
				cut = 2;
			}

			if (remainingTxt[found + 1] == 'h') {
				hot = !hot;
				styleChange = styleHot;
				if (remainingTxt[found + 2] == '{') {
					size_t end = remainingTxt.find("}", found);
					std::string id = remainingTxt.substr(found + 3, end - (found + 3));
					tagId = std::stoi(id);
					cut = 4 + id.size();
				}
				else {
					cut = 2;
				}
			}
			//other markup checks here



		}

		writeTxt = remainingTxt.substr(0, found);
		remainingTxt = remainingTxt.substr(writeTxt.size() + cut, std::string::npos);

		pWin->appendText(writeTxt);

		if (styleChange == styleBold)
			pWin->setAppendStyleBold(bold);
		if (styleChange == styleHot)
			pWin->setAppendStyleHot(hot, tagId);

	}
}

/** If any registered hot text is found in the given text, mark it up for further processing.*/
void CWorldUI::markupHotText(std::string & text) {
	for (auto hotText : hotTextList) {
		size_t found = text.find(hotText.text);
		while (found != std::string::npos) {
			if ((found > 0 && !isalnum(text[found - 1])) &&
				(!isalnum(text[found + hotText.text.size()]))) {
				//std::string tag = "\\h{" + std::to_string(hotText.id) + "}";
				//text.insert(found, tag);
				//text.insert(found + tag.size() + hotText.text.size(), "\\h");
				std::string tag = makeHotText(hotText.text, hotText.id);
				text.replace(found, hotText.text.size(), tag);
				found += tag.size();
			}
			found = text.find( hotText.text, found + hotText.text.size());
		}
	}
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
	//take(hotId-1);
	if (hotId < memberIdStart) {
		int objId = localHotList.getObjectId(hotId);
		objectClick(objId,mousePos);
	}
	
}

/** Handle a click on an inventory window item with the given object id. */
void CWorldUI::inventoryClick(int hotId) {
	int objId = localHotList.getObjectId(hotId);
	drop(objId);
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
	std::string takeText = "\nI picked up the ";
	takeText += makeHotText(pVM->ObjMessage(objId, "name").getStringValue(),clickedHotText);
	processText(takeText + ".");
	move(objId, playerId);
	refreshInvWindow();
}

/** Drop this object. */
void CWorldUI::drop(int objId) {
	move(objId, currentRoomNo);
	refreshInvWindow();
	std::string dropText = "\nI dropped the ";
	dropText += makeHotText(pVM->ObjMessage(objId, "name").getStringValue(), clickedHotText);
	processText(dropText + ".");
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
	setCurrentWindow(pInvWindow);
	processText("Inventory:\n");
	std::string invText;
	int item = child(playerId);
	if (item) {
		do {
			int localId = localHotList.getLocalId(item);
			//invText += "\\h{" + std::to_string(localId) + "}";
			//invText += "\nA ";
			//invText += pVM->ObjMessage(item, "name").getStringValue();
			//invText += "\\h";
			invText += makeHotText("\nA " + pVM->ObjMessage(item, "name").getStringValue(), localId);
		} while (objectInLoop(playerId, item));
		processText(invText);
	}
	setCurrentWindow(pTextWindow);
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
	popChoices.clear();
	//acquire the different options available
	popChoices.push_back({ "Do nothing",popDoNothing });
	//is it on the ground? We can take it
	if (parent(objId) == currentRoomNo)
		popChoices.push_back({ "Take", popTake });
	if (parent(objId) == playerId)
		popChoices.push_back({ "Drop", popDrop });
	popChoices.push_back({ "Examine", popExamine });

	showPopupMenu(mousePos);
}

void CWorldUI::showPopupMenu(const glm::i32vec2& mousePos) {
	pPopMenu->clear();
	pPopMenu->setPos(mousePos.x, mousePos.y);
	for (auto item : popChoices) {
		pPopMenu->addItem(item.actionText);
	}
	pPopMenu->setVisible(true);
	pPopMenu->makeModal(pPopMenu);
}

std::string CWorldUI::makeHotText(std::string text, int idNo) {
	std::string hotStr = "\\h{" + std::to_string(idNo) + "}";
	hotStr += text + "\\h";
	return hotStr;
}

/** Respond to the user selecting an item from the popup menu. */
void CWorldUI::popupSelection(int choice) {
	TPopAction action = popChoices[choice].action;
	if (action == popTake)
		take(clickedObj);
	if (action == popDrop)
		drop(clickedObj);
}

