#include "inventoryWin.h"



#include "../gameTextWin.h"
#include "../gameState.h"

#include "../items/item.h"

void CInventoryWin::refresh() {
	return;
	//scrapping so scrap

	if (gameWorld.player == nullptr)
		return;
	//TO DO: messy! Shouldn't have to keep making that check here

	pWin->clearText();
	for (auto& item : gameWorld.player->playerC->inventory) {
		pWin->addText(item->getShortDesc());
		pWin->addText("\n");
	}
}
