#include "itemMenu.h"

#include "../items/item.h"
#include "../gameState.h"
#include "../gameTextWin.h"
#include "../gameGui.h"

void CItemMenu::update(float dT) {
	if (pWin->visible == false)
		return;

	if (timer > 0 && !pWin->isMouseOver()) {
		timer -= dT;
		if (timer < 0) {
			pWin->setVisible(false);
			currentEntityNo = 0;
		}
	}
}

/** Catch a click on drop/examine, etc. */
void CItemMenu::onRichTextClick(const std::string& msg) {
	if (msg == "drop") {
		game.player->dropItem(currentEntityNo);
		currentEntityNo = 0;
	} 
	else if (msg == "examine") {
		//int entityNo = getEntityNo(msg);
		CItem* item = (CItem*)game.map->getEntity(currentEntityNo);
		item->examine();
	}
	pWin->setVisible(false);
}

void CItemMenu::onMsg(const std::string& msg) {
	if (msg == "timeout") {
		if (timer <= 0) 
			timer = 0.5f;
		return;
	}

	timer = 0;

	int entityNo = getEntityNo(msg);

	if (entityNo == currentEntityNo) {
		return;
	}

	currentEntityNo = entityNo;
	CItem* item = (CItem * )game.map->getEntity(entityNo);
	std::string src = msg.substr(0, msg.find(" ")).c_str();

	//position next to source window
	gWin::putLeftOf("itemMenu", "inv");

	//get this item's menu text
	std::string menuTxt;
	if (src == "inv") {
		menuTxt = item->getMenuTextInv();

	}
	//else
	//	menuTxt = item->getMenuTextNear();


	//display it
	pWin->clearText();
	pWin->addText(menuTxt);

	gWin::alignWithMouse("itemMenu");
	pWin->setVisible(true);


}