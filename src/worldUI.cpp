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
	findClassIds();
	clickedHotText = 0;
	backDirection = moveNone;
}

void CWorldUI::findMoveToIds() {
	moveToIds[moveNone] = 0;
	moveToIds[moveNorth] = pVM->getMemberId("northTo");
	moveToIds[moveNE] = pVM->getMemberId("neTo");
	moveToIds[moveEast] = pVM->getMemberId("eastTo");
	moveToIds[moveSE] = pVM->getMemberId("seTo");
	moveToIds[moveSouth] = pVM->getMemberId("southTo");
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

void CWorldUI::findClassIds() {
	corridorClassId = pVM->getGlobalVar("corridorClassId").getIntValue();
	roomClassId = pVM->getGlobalVar("roomClassId").getIntValue();
	staticClassId = pVM->getGlobalVar("staticClassId").getIntValue();
}

/** Compiles the current room's description and sends it for display. */
void CWorldUI::roomDescription() {
	std::string description = pVM->objMessage(currentRoom, "description").getStringValue() + " ";
	description = markupHotText(description);
	pTextWindow->appendMarkedUpText(description);

	for (int dir = moveNorth; dir <= moveOut; dir++) {
		bodyListedExits[dir] = pTextWindow->isActiveHotText(moveToIds[dir]);
	}

	description = getExitsText(currentRoomNo);
	description = markupHotText(description);
	pTextWindow->appendMarkedUpText(description);

	//List any contents
	refreshLocalList();
	refreshInvWindow();
	int item = child(currentRoomNo);
	if (!item)
		return;
	std::string itemsText;
	std::vector<int> otherItems;
	do {
		if (pVM->getClass(item) == staticClassId) {
			itemsText += pVM->objMessage(item, "initial").getStringValue();
		}
		else
			otherItems.push_back(item);

	} while (objectInLoop(currentRoomNo, item));

	if (otherItems.size() > 0) {
		itemsText += "\n\nI could also see";
		for (unsigned int idx = 0; idx < otherItems.size(); idx++) {
			int itemNo = otherItems[idx];
			int localId = localHotList.addObject(itemNo);
			itemsText += makeHotText(" a " + pVM->objMessage(itemNo, "name").getStringValue(), localId);
			if (idx < otherItems.size() - 2)
				itemsText += ",";
			if (idx == otherItems.size() - 2)
				itemsText += " and";
		}
		itemsText += " here.";
	}
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
		//if this is a direction, check we can actually go there:
		if (hotText.id >= moveToIds[moveNorth] && hotText.id <= moveToIds[moveOut]
			&& pVM->getMemberValue(currentRoomNo, hotText.id) == 0) {
			continue;
		}
		size_t found = text.find(hotText.text);
		while (found != std::string::npos) {
		
			if ((found > 0 && !isalnum(text[found - 1])) && (!isalnum(text[found + hotText.text.size()]))) {
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
	for (int dir = moveNorth; dir <= moveOut; dir++) {
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
		backDirection = calcBackDirection(moveId);
		currentRoom = member;
		currentRoomNo = member.getObjId();
		pTextWindow->purgeHotText();
		pTextWindow->appendText("\n\n");
		roomDescription();
	}
}

/** Return the opposite direction. */
TMoveDir CWorldUI::calcBackDirection(int moveId) {
	int dir;
	for ( dir = moveNorth; dir <= moveOut; dir++) {
		if (moveToIds[dir] == moveId)
			break;
	}

	if (dir < moveUp) {
		dir += 4;
		return static_cast<TMoveDir>(dir % 8);
	}

	switch (dir) {
	case moveUp: return moveDown;
	case moveDown: return moveUp;
	case moveIn: return moveOut;
	case moveOut: return moveIn;
		}

	

}


/** Pick up this object. */
void CWorldUI::take(int objId) {
	pTextWindow->purgeHotText(clickedHotText);
	std::string takeText = "\n\nI picked up the ";
	takeText += makeHotText(pVM->objMessage(objId, "name").getStringValue(), localHotList.getLocalId(objId));
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
	dropText += makeHotText(pVM->objMessage(objId, "name").getStringValue(), localId);
	dropText = markupHotText(dropText + ".");
	pTextWindow->appendMarkedUpText(dropText);
}

void CWorldUI::examine(int objId) {
	int currentRextent = popControl->drawBox.pos.x + popControl->getWidth();
	popControl->clear();
	popControl->resize(300, 200);
	popControl->setTextStyle(popHeaderStyle);
	
	std::string examText  = cap(pVM->objMessage(objId, "name").getStringValue())
		+ "\n";
	examText = markupHotText(examText);
	popControl->appendMarkedUpText(examText);
	popControl->setTextStyle(popBodyStyle);
	examText = pVM->objMessage(objId, "description").getStringValue();
	examText = markupHotText(examText);

	popControl->appendMarkedUpText(examText);
//	popControl->appendText("\n");

	//add convenience options, ie, take/drop
	popChoices.clear();
	if (parent(objId) == currentRoomNo) {
		if (pVM->getClass(objId) != staticClassId)
			popChoices.push_back({ "\nTake", popTake });
	}
	if (parent(objId) == playerId) {
		popChoices.push_back({ "\nDrop", popDrop });
	}
	//popChoices.push_back({ "\nDo nothing", popDoNothing });
	appendChoicesToPopup();

	showPopupMenu(popControl->drawBox.pos);
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
			invText += makeHotText("\nA " + pVM->objMessage(item, "name").getStringValue(), localId);
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
	popChoices.push_back({ "Examine", popExamine });
	if (parent(objId) == currentRoomNo) {
		if (pVM->getClass(objId) != staticClassId)
			popChoices.push_back({ "\nTake", popTake });
	}
	if (parent(objId) == playerId)
		popChoices.push_back({ "\nDrop", popDrop });
	//popChoices.push_back({ "\nDo nothing",popDoNothing });
	
	appendChoicesToPopup();

	
	showPopupMenu(mousePos);
}

void CWorldUI::appendChoicesToPopup() {
	string popStr; int choiceNo = 1;
	for (auto item : popChoices) {
		popStr += makeHotText(item.actionText, choiceNo);
		//if (choiceNo < popChoices.size())
		//	popStr += "\n";
		choiceNo++;
	}
	markupHotText(popStr);
	popControl->appendMarkedUpText(popStr);
}

/** Display the popup menu at the cursor position, adjusted for its dimensions and the screen edge. */
void CWorldUI::showPopupMenu(const glm::i32vec2& cornerPos) {
	popControl->resizeToFit();
	glm::i32vec2 newCornerPos = cornerPos + glm::i32vec2(0, pTextWindow->getFont()->lineHeight  );
	int margin = 30;
	if (newCornerPos.x + popControl->getWidth() + margin > popControl->parent->getWidth())
		//newCornerPos.x = cornerPos.x - popControl->getWidth();
		newCornerPos.x -= (newCornerPos.x + popControl->getWidth() + margin) - popControl->parent->getWidth();


	if (newCornerPos.y + popControl->getHeight() + margin > popControl->parent->getHeight())
		newCornerPos.y -= (newCornerPos.y + popControl->getHeight() + margin) - popControl->parent->getHeight();

	popControl->setPos(newCornerPos.x, newCornerPos.y);
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

/** Return a string describing the exits from this room. */
std::string CWorldUI::getExitsText(int roomNo) {
	std::string exitsText; 
	struct TExit {
		int directionId;
		int destination;
		std::string directionStr;
		std::string destinationStr;
	};
	std::vector < TExit> exits;
	std::vector < TExit> corridors;

	//loop through the exits, gathering information
	for (int dir = moveNorth; dir <= moveOut; dir++) {
		int destination = pVM->getMemberValue(roomNo, moveToIds[dir]);
		if (!destination || bodyListedExits[dir])
			continue;
		int directionId = moveToIds[dir];
		std::string destinationStr = pVM->objMessage(destination, "shortName").getStringValue();
		auto directionHottext = find_if(hotTextList.begin(), hotTextList.end(),
			[&](THotTextRec& hotRec) { return hotRec.id == directionId; });
		if (pVM->getClass(destination) == corridorClassId)
			corridors.push_back({ directionId,destination,directionHottext->text,destinationStr });
		else
			exits.push_back({ directionId,destination,directionHottext->text,destinationStr });
	}

	int noCorridors = corridors.size();
	if (noCorridors == 1)
		exitsText = "A " + corridors[0].destinationStr + " led " + corridors[0].directionStr + ". ";
	if (noCorridors > 1) {
		exitsText = "Corridors led " + corridors[0].directionStr;
		for (int corridorNo = 1; corridorNo < noCorridors - 1; corridorNo++) {
			exitsText += ", " + corridors[corridorNo].directionStr;
		}
		exitsText += " and " + corridors[noCorridors - 1].directionStr + ". ";
	}

	int noExits = exits.size();
	if (noExits == 1) {
		if (exits[0].directionId == moveToIds[backDirection])
			exitsText += "The way back led " + exits[0].directionStr + ". ";
		else
			exitsText += "A doorway opened to the " + exits[0].directionStr + ". ";
	}

	if (noExits > 1) {
		exitsText += "Doorways opened to the " + exits[0].directionStr;
		for (int exitNo = 1; exitNo < noExits - 2; exitNo++) {
			exitsText += ", " + exits[exitNo].directionStr;
		}
		exitsText += " and " + exits[noExits - 1].directionStr + ". ";
	}

	return exitsText;
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

