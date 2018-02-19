#include "worldUI.h"

void CWorldUI::setVM(CTigVM * vm) {
	pVM = vm;
}

void CWorldUI::init() {
	pVM->execute();
	currentRoom = pVM->getGlobalVar("startRoom");
	if (currentRoom.type != tigObj) {
		std::cerr << "\nStart room not found!";
		return;
	}
}

/** Execute the current room's description member. */
void CWorldUI::roomDescription() {
	pVM->ObjMessage(currentRoom, "description");
}

/** Start a game session. */
void CWorldUI::start() {
	roomDescription();
}

/** Add this as a recognised hot text word. */
void CWorldUI::addHotText(std::string & text, int memberId) {
	THotTextRec hotText;
	hotText.text = text;
	hotText.memberId = memberId;
	hotTextList.push_back(hotText);
}
