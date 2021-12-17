#include "inventoryWin.h"


#include "../gameGui.h"
#include "../gameTextWin.h"


void CInventoryWin::onRichTextMouseOver(const std::string& mouseOn) {
	if (  mouseOn == "" && currentItem != "" ) {
		gWin::msg("itemMenu", "timeout");
		currentItem = "";
		return;
	}

	if (mouseOn == "") {
		currentItem = "";
		return;
	}

	currentItem = mouseOn;

	//open itemMenu for this item
	gWin::msg("itemMenu","inv " + mouseOn);
}

void CInventoryWin::onMouseOff() {
	gWin::msg("itemMenu", "timeout");
}

void CInventoryWin::update(float dT) {
	//if (!currentItem.empty() && !pWin->isMouseOver()) {
	//	gWin::msg("itemMenu", "timeout");
	//	currentItem = "";
	//}

	if (pWin->isMouseOver() == false)
		gWin::msg("itemMenu", "timeout");
}
