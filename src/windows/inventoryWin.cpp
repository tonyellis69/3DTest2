#include "inventoryWin.h"


#include "../gameGui.h"
#include "../gameTextWin.h"


void CInventoryWin::onRichTextMouseOver(const std::string& mousedItem) {
	return;

	if (  mousedItem == "" && currentItem != "" ) {
		gWin::msg("itemMenu", "timeout");
		currentItem = "";
		return;
	}

	if (mousedItem == "") {
		currentItem = "";
		return;
	}

	currentItem = mousedItem;

	//open itemMenu for this item
	gWin::msg("itemMenu","inv " + mousedItem);
}



void CInventoryWin::onEvent(CEvent& e) {
	if (e.type == eHotTextHover) {
		if (std::get<std::string>(e.data).size() == 0)
			gWin::msg("itemMenu", "timeout");
		else
			gWin::msg("itemMenu", "inv " + std::get<std::string>(e.data) );

	}
	else if (e.type == eMouseOff && e.id == pWin->richText->uniqueID) {
		gWin::msg("itemMenu", "timeout");
	}
}

void CInventoryWin::update(float dT) {
	//if (pWin->isMouseOver() == false)
	//	gWin::msg("itemMenu", "timeout");
}
