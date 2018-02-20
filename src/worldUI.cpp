#include "worldUI.h"

void CWorldUI::setVM(CTigVM * vm) {
	pVM = vm;
}

void CWorldUI::setTextWindow(CGUIrichText * txtWin) {
	pTextWindow = txtWin;
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


/** Add this as a recognised hot text expression. */
void CWorldUI::addHotText(std::string & text, int memberId) {
	THotTextRec hotText;
	hotText.text = text;
	hotText.memberId = memberId;
	hotTextList.push_back(hotText);
}

/** Checking for hot text and style markups, turn the given text into one or more rich-text
	instructions sent to the text control. */
void CWorldUI::processText(string& text) {
	markupHotText(text);

	bool bold = false; bool hot = false;
	enum TStyleChange { styleNone, styleBold,styleHot };

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
					size_t end = remainingTxt.find("}",found);
					std::string id = remainingTxt.substr(found + 3, end - (found +3));
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

		pTextWindow->appendText(writeTxt);

		if (styleChange == styleBold)
			pTextWindow->setAppendStyleBold(bold);
		if (styleChange == styleHot)
			pTextWindow->setAppendStyleHot(hot,tagId);

	}
}

/** If any hot text is found in the given text, mark it up for further processing.*/
void CWorldUI::markupHotText(std::string & text) {
	for (auto hotText : hotTextList) {
		size_t found = text.find(hotText.text);
		if (found != std::string::npos) {
			std::string tag = "\\h{" + std::to_string(hotText.memberId) + "}";
			text.insert(found, tag);
			text.insert(found + tag.size() + hotText.text.size(), "\\h");
		}
	}

}
