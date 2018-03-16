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

void CWorldUI::init() {
	pVM->execute();
	currentRoom = pVM->getGlobalVar("startRoom"); //TO DO: scrap
	currentRoomNo = currentRoom.getObjId();
	playerId = pVM->getGlobalVar("playerObj").getObjId();
	findMoveToIds();
	findTreeIds();
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

/** Execute the current room's description member. */
void CWorldUI::roomDescription() {
	setCurrentWindow(pTextWindow);
	pVM->ObjMessage(currentRoom, "description");

	//List any contents
	roomItems.clear();
	int item = child(currentRoomNo);
	if (!item)
		return;
	processText("\n\nI could see");
	do {
		roomItems.push_back(item);
		processText("\\h{" + std::to_string(roomItems.size()) + "}");
		processText(" a ");
		pVM->ObjMessage(item, "name");
		processText("\\h");
		if (sibling(item)) {
			if (sibling(sibling(item)))
				processText(",");
			else
				processText(" and");
		}
	} while (objectInLoop(currentRoomNo, item));
	processText(" here.");
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
				std::string tag = "\\h{" + std::to_string(hotText.id) + "}";
				text.insert(found, tag);
				text.insert(found + tag.size() + hotText.text.size(), "\\h");
				found += tag.size() + 2;
			}
			found = text.find( hotText.text, found + hotText.text.size());
		}
	}
}

/** Handle the player clicking on a piece of hot text. */
void CWorldUI::hotTextClick(int hotId) {
	//is this a move command?
	for (int dir = 0; dir < 12; dir++) {
		if (hotId == moveToIds[dir]) {
			changeRoom(hotId);
			return;
		}
	}

	//is it a click on an item in the room description? check range
	take(hotId-1);
	
}

/** Handle a click on an inventory window item with the given object id. */
void CWorldUI::inventoryClick(int hotId) {
	drop(hotId);
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
void CWorldUI::take(int itemIndex) {
	int itemId = roomItems[itemIndex];
	pTextWindow->purgeHotText(itemIndex+1);
	processText("\nI picked up the ");
	pVM->ObjMessage(itemId, "name");
	processText(".");
	roomItems[itemIndex] = 0;
	move(itemId, playerId);
	refreshInvWindow();
}

/** Drop this object. */
void CWorldUI::drop(int itemId) {
	move(itemId, currentRoomNo);
	refreshInvWindow();
	processText("\nI dropped the ");
	pVM->ObjMessage(itemId, "name");
	processText(".");
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

	int item = child(playerId);
	if (item)
		do {
			processText("\\h{" + std::to_string(item) + "}");
			processText("\nA ");
			pVM->ObjMessage(item, "name");
			processText("\\h");
		} while (objectInLoop(playerId, item));
	setCurrentWindow(pTextWindow);
}

