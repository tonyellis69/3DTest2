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
	currentRoomNo = pVM->getGlobalVar("startRoom").getObjId(); //TO DO: scrap
	currentRoom = pVM->getObject(currentRoomNo);
	playerId = pVM->getGlobalVar("playerObj").getObjId();
	player = pVM->getObject(playerId);
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
	sceneryClassId = pVM->getGlobalVar("sceneryClassId").getIntValue();
	supporterClassId = pVM->getGlobalVar("supporterClassId").getIntValue();
}

/** Compiles the current room's description and sends it for display. */
void CWorldUI::roomDescription() {
	refreshLocalList();
	refreshInvWindow();

	//go through room contents
	CObjInstance* item = child(currentRoom);
	std::string itemsText;
	std::vector<CObjInstance*> otherItems; auto sceneryStart = hotTextList.size();
	if (item) {
		do {
			if (pVM->inheritsFrom(item, sceneryClassId)) {
				//register the object's name as hot text
				std::string name = pVM->objMessage(item->id, "name").getStringValue();
				int localId = localHotList.addObject(item);
				hotTextList.push_back({ name,localId });
				continue;
			}
			if (pVM->inheritsFrom(item, staticClassId) || (pVM->getMemberValue(item->id, "moved") == 0)) {
				itemsText += "\n\n" + markupInitialText(item);
			}
			else
				otherItems.push_back(item);
		} while (objectInLoop(currentRoom, item));


		if (otherItems.size() > 0) {
			itemsText += "\n\nI could also see";
			for (unsigned int idx = 0; idx < otherItems.size(); idx++) {
				item = otherItems[idx];
				int localId = localHotList.addObject(item);
				itemsText += makeHotText(" a " + pVM->objMessage(item, "name").getStringValue(), localId,item->id);
				if (idx < otherItems.size() - 2 && otherItems.size() > 1)
					itemsText += ",";
				if (idx == otherItems.size() - 2)
					itemsText += " and";
			}
			itemsText += " here.";
		}
	}

	std::string description = pVM->objMessage(currentRoom, "description").getStringValue() + " ";
	description = markupExits(description);
	pTextWindow->appendMarkedUpText(description);

	for (int dir = moveNorth; dir <= moveOut; dir++) { //note any exits already mentioned in room description.
		bodyListedExits[dir] = pTextWindow->isActiveHotText(moveToIds[dir]);
	}
	std::string exitsText = getExitsText(currentRoom);
	exitsText = markupExits(exitsText);

	pTextWindow->appendMarkedUpText(exitsText);
	pTextWindow->appendMarkedUpText(itemsText);

	if (hotTextList.size() > sceneryStart)
		hotTextList.erase(hotTextList.begin() + sceneryStart, hotTextList.end());
}

/**Return the initial description of this object, with the name text marked up as hot text.*/
std::string CWorldUI::markupInitialText(CObjInstance* obj) {
	std::string initialText = pVM->objMessage(obj, "initial").getStringValue();
	std::string nameText = pVM->objMessage(obj, "name").getStringValue();
	
	size_t found = initialText.find(nameText);

	if (found != string::npos) {
		int localId = localHotList.addObject(obj);
		std::string hotText = makeHotText(nameText, localId, obj->id);
		initialText.replace(found, nameText.size(), hotText);
	}
	return initialText;
	//TO DO: maybe check object for other instructions about hot text.
}

/** Start a game session. */
void CWorldUI::start() {
	roomDescription();
}


/** Add this as a recognised hot text expression. */
void CWorldUI::addHotText(std::string & text, int msgId, int objId) {
	THotTextRec hotText;
	hotText.text = text;
	hotText.msgId = msgId;
	hotText.objId = objId;
	hotTextList.push_back(hotText);
}


/** If any registered hot text is found in the given text, mark it up for further processing.*/
std::string CWorldUI::markupExits(std::string & text) {
	for (auto hotText : hotTextList) {
		//if this is a direction, check we can actually go there:
		if (hotText.msgId >= moveToIds[moveNorth] && hotText.msgId <= moveToIds[moveOut]
			&& pVM->getMemberValue(currentRoomNo, hotText.msgId) == 0) {
			continue;
		}
		size_t found = text.find(hotText.text);
		while (found != std::string::npos) { //check found text isn't part of a bigger word:
			if ((found == 0 || !isalnum(text[found - 1])) && !isalnum(text[found + hotText.text.size()]) ) {
				std::string tag = makeHotText(hotText.text, hotText.msgId, currentRoom->id);
				text.replace(found, hotText.text.size(), tag);
				found += tag.size();
			}
			found = text.find( hotText.text, found + hotText.text.size());
		}
	}
	return text;
}

/** Handle the player clicking on a piece of hot text. */
void CWorldUI::hotTextClick(int msgId, int objId, glm::i32vec2 mousePos) {
	clickedHotText = msgId;
	//is this a move command?
	for (int dir = moveNorth; dir <= moveOut; dir++) {
		if (msgId == moveToIds[dir]) {
			changeRoom(msgId);
			return;
		}
	}

	//is it a click on an item in the room description? check range
	if (msgId < memberIdStart) {
		//int obj = localHotList.getObjectId(msgId);
		objectClick(objId,mousePos);
	}
}

/** Handle a click on an inventory window item with the given object id. */
void CWorldUI::inventoryClick(int msgId, int objId, const glm::i32vec2& mousePos) {
//	int obj = localHotList.getObjectId(msgId);
	objectClick(objId, mousePos);
}

/** Change current room. */
void CWorldUI::changeRoom(int moveId) {
	CTigVar member = pVM->getMember(currentRoom, moveId);
	if (member.type == tigObj) {
		backDirection = calcBackDirection(moveId);
		currentRoomNo = member.getObjId();
		currentRoom = pVM->getObject(currentRoomNo);
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
	CObjInstance* obj = pVM->getObject(objId);
	pTextWindow->purgeHotText(clickedHotText);
	std::string takeText = "\n\nI picked up the ";
	takeText += makeHotText(pVM->objMessage(obj, "name").getStringValue(), localHotList.getLocalId(obj),obj->id);
	pTextWindow->appendMarkedUpText(takeText + ".");
	move(obj, player);
	pVM->setMemberValue(obj->id, "moved", CTigVar(1));
	refreshInvWindow();
}

/** Drop this object. */
void CWorldUI::drop(int objId) {
	CObjInstance* obj = pVM->getObject(objId);
	int localId = localHotList.getLocalId(obj);
	pTextWindow->purgeHotText(localId);
	move(obj, currentRoom);
	refreshInvWindow();
	std::string dropText = "\n\nI dropped the ";
	dropText += makeHotText(pVM->objMessage(obj, "name").getStringValue(), localId, obj->id);
	pTextWindow->appendMarkedUpText(dropText + ".");
}

void CWorldUI::examine(int objId) {
	CObjInstance* obj = pVM->getObject(objId);
	int currentRextent = popControl->drawBox.pos.x + popControl->getWidth();
	popControl->clear();
	popControl->resize(300, 200);
	popControl->setTextStyle(popHeaderStyle);
	
	std::string examText  = cap(pVM->objMessage(obj, "name").getStringValue())
		+ "\n";
	popControl->appendMarkedUpText(examText);
	popControl->setTextStyle(popBodyStyle);
	examText = pVM->objMessage(obj, "description").getStringValue();

	if (pVM->inheritsFrom(obj, supporterClassId)) {
		examText += " " + pVM->objMessage(obj, "search").getStringValue();
	}

	popControl->appendMarkedUpText(examText);

	//add convenience options, ie, take/drop
	popChoices.clear();
	if (parent(obj) == currentRoom) {
		CObjInstance* staticClass = pVM->getObject(staticClassId);
		if (!pVM->inheritsFrom(obj, staticClass))
			popChoices.push_back({ "\nTake", popTake });
	}
	if (parent(obj) == player) {
		popChoices.push_back({ "\nDrop", popDrop });
	}
	//popChoices.push_back({ "\nDo nothing", popDoNothing });
	appendChoicesToPopup(objId);

	showPopupMenu(popControl->drawBox.pos);
}

void CWorldUI::push(int objId) {
	CObjInstance* obj = pVM->getObject(objId);
	std::string result = pVM->objMessage(obj, "push").getStringValue();
	pTextWindow->appendMarkedUpText(result);
}


/** Return the index of first child of the given parent, if any. */
int CWorldUI::child(int parent) {
	return pVM->getMemberValue(parent, childId);
}

CObjInstance* CWorldUI::child(CObjInstance* parentObj) {
	int result = parentObj->members[childId].getObjId();
	if (result)
		return pVM->getObject(result);
	else
		return NULL;
}

/** Return index of sibling of given object, if any. */
int CWorldUI::sibling(int object) {
	return pVM->getMemberValue(object, siblingId);
}

CObjInstance* CWorldUI::sibling(CObjInstance* obj) {
	int result = obj->members[siblingId].getObjId();
	if (result)
		return pVM->getObject(result);
	else
		return NULL;
}

/** Return index of parent of given object, if any. */
int CWorldUI::parent(int childNo) {
	return pVM->getMemberValue(childNo, parentId);
}

CObjInstance* CWorldUI::parent(CObjInstance* child) {
	int result = child->members[parentId].getObjId();
	if (result)
		return pVM->getObject(result);
	else
		return NULL;
}

/** Return true while parent has a descendant and child = 0 or a decendant with a sibling .*/
bool CWorldUI::objectInLoop(int parent, int& childNo) {
	if (childNo == 0) 
		childNo = child(parent);
	else
		childNo = sibling(childNo);
	return (bool)childNo;
}

bool CWorldUI::objectInLoop(CObjInstance* parent, CObjInstance* &childObj) {
	if (childObj == NULL)
		childObj = child(parent);
	else
		childObj = sibling(childObj);
	return (bool)childObj;
}

/** Make the given object a child of the destination object. */
void CWorldUI::move(CObjInstance* obj, CObjInstance* dest) {
	CObjInstance* parentObj = parent(obj);
	CObjInstance* childObj = child(parentObj); CObjInstance* olderSibling = NULL;
	while (childObj != obj) {
		olderSibling = childObj;
		childObj = sibling(childObj);
	}
	if (olderSibling)
		olderSibling->members[siblingId] = obj->members[siblingId];
	else
		parentObj->members[childId] = obj->members[siblingId];

	CObjInstance* destChild = child(dest);
	if (destChild) {
		obj->members[siblingId].setObjId(destChild->id);
	} else
		obj->members[siblingId].setObjId(NULL);
	dest->members[childId].setObjId(obj->id);
	obj->members[parentId].setObjId(dest->id);
}

/** Ensure inventory window shows the latest player contents. */
void CWorldUI::refreshInvWindow() {
	pInvWindow->clear();
	pInvWindow->appendMarkedUpText("Inventory:\n");
	std::string invText;
	CObjInstance* item = child(player);
	if (item) {
		do {
			int localId = localHotList.getLocalId(item);
			invText += makeHotText("\nA " + pVM->objMessage(item, "name").getStringValue(), localId, item->id);
		} while (objectInLoop(player, item));
		pInvWindow->appendMarkedUpText(invText);
	}
}

/** Clear the local list and repopulate it with items the player is carrying. */
void CWorldUI::refreshLocalList() {
	localHotList.clear();
	CObjInstance* item = child(player);
	if (item)
		do {
			localHotList.addObject(item);
		} while (objectInLoop(player, item));
}

/** Handle a user-click on this object. */
void CWorldUI::objectClick(int objId, const glm::i32vec2& mousePos) {
	popControl->clear();
	CObjInstance* obj = pVM->getObject(objId);

	popChoices.clear();
	popControl->resize(200, 200);
	//acquire the different options available
	//is it on the ground? We can take it
	popChoices.push_back({ "Examine", popExamine });
	if (parent(obj) == currentRoom) {
		CObjInstance* staticClass = pVM->getObject(staticClassId);
		if (!pVM->inheritsFrom(obj,staticClass) )
			popChoices.push_back({ "\nTake", popTake });
	}
	if (parent(obj) == player)
		popChoices.push_back({ "\nDrop", popDrop });
	//popChoices.push_back({ "\nDo nothing",popDoNothing });

	//check for additional properties, such as being pushable.
	int pushId = pVM->getMemberId("push");
	if (pVM->hasMember(obj, pushId)) {
		popChoices.push_back({ "\nPush", popPush });
	}

	appendChoicesToPopup(objId);	
	showPopupMenu(mousePos);
}

void CWorldUI::appendChoicesToPopup(int objId) {
	string popStr; int choiceNo = 1;
	for (auto item : popChoices) {
		popStr += makeHotText(item.actionText, choiceNo, objId);
		choiceNo++;
	}
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

std::string CWorldUI::makeHotText(std::string text, int msgId, int objId) {
	std::string hotStr = "\\h{" + std::to_string(msgId) + '@' + std::to_string(objId) + "}";
	hotStr += text + "\\h";
	return hotStr;
}

/** Respond to the user selecting an item from the popup menu. */
void CWorldUI::popupSelection(int choice, int objId, glm::i32vec2& mousePos) {
	currentMousePos = mousePos;
	if (choice == -1)
		return;
	TPopAction action = popChoices[choice-1].action;
	switch (action) {
		case popTake: take(objId); break;
		case popDrop: drop(objId); break;
		case popExamine: examine(objId); break;
		case popPush: push(objId); break;
	}
}

std::string CWorldUI::cap(std::string text) {
	text[0] = toupper(text[0]);
	return text;
}

/** Return a string describing the exits from this room. */
std::string CWorldUI::getExitsText(CObjInstance * roomObj) {
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
		int destination = roomObj->members[moveToIds[dir]].getIntValue();// pVM->getMemberValue(room, moveToIds[dir]);
		if (!destination || bodyListedExits[dir])
			continue;
		int directionId = moveToIds[dir];
		std::string destinationStr = pVM->objMessage(destination, "shortName").getStringValue();
		auto directionHottext = find_if(hotTextList.begin(), hotTextList.end(),
			[&](THotTextRec& hotRec) { return hotRec.msgId == directionId; });
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

