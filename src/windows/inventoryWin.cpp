#include "inventoryWin.h"



#include "../gameTextWin.h"
#include "../gameState.h"


void CInventoryWin::refresh() {
	return;
	//scrapping so scrap

	if (game.player == nullptr)
		return;
	//TO DO: messy! Shouldn't have to keep making that check here

	pWin->clearText();
	for (auto& item : game.player->inventory) {
		pWin->addText(item->getShortDesc());
		pWin->addText("\n");
	}
}
