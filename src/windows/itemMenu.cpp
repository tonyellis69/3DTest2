#include "itemMenu.h"

#include "../items/item.h"
#include "../gameState.h"
#include "../gameTextWin.h"
#include "../gameGui.h"

#include "UI/gui2.h"

#include "win/win.h"

#include "utils/log.h"

void CItemMenu::onEvent(CGUIevent& e) {
	if (e.type == eHotTextHover && (UI.mouseIn("inv") || UI.mouseIn("near")) ) {
		if (e.hotTxt->empty()) 
			startTimeout();
		else {
			currentItem = std::stoi(e.hotTxt->substr(e.hotTxt->find_first_of("0123456789")).c_str());
			showItemChoices();
		}
	}
	else if (e.type == eMouseOff  && UI.mouseNotIn("itemMenu")
		 && UI.mouseNotIn("itemMenu") ) {
		startTimeout();
	}
	else if (e.type == eLeftClick && UI.mouseIn("itemMenu" )) {
		onRichTextClick(*e.hotTxt);
	}
}

void CItemMenu::startTimeout() {
	if (timer <= 0 && pWin->visible)
		timer = 0.5f;
}

void CItemMenu::showItemChoices() {
	CItem* item = (CItem*)game.getEntity(currentItem);

	//get this item's menu text
	std::string menuTxt;

	if (UI.mouseIn("inv")) {
		positionLeftOf(UI.findControl("inv"));
		menuTxt = item->getMenuTextInv();
	}
	else {
		positionLeftOf(UI.findControl("near"));
		menuTxt = item->getMenuTextNear();
	}


	

	//display it
	pWin->clearText();
	pWin->addText(menuTxt);

	pWin->setVisible(true);
	timer = 0;
}


void CItemMenu::positionLeftOf(CguiBase* spawner) {
	//find left edge of parentEntity
	glm::i32vec2 pos = spawner->getLocalPos();
	glm::i32vec2 size = pWin->getSize();

	//position back from that
	pWin->anchorLeft = NONE;
	pWin->setPosX(pos.x - size.x - 10);

	alignWithMouse(spawner);
}

void CItemMenu::alignWithMouse(CguiBase* spawner) {
	//find mouse y pos
	glm::i32vec2 mousePos;
	CWin::getMousePos(mousePos.x, mousePos.y);

	glm::i32vec2 winSize = pWin->getSize();
	int ypos = mousePos.y - (winSize.y / 2);
	if (ypos < spawner->getLocalPos().y)
		ypos = spawner->getLocalPos().y;

	pWin->setPosY(ypos);
}

void CItemMenu::update(float dT) {
	if (pWin->visible == false )
		return;

	if (timer > 0 ) { 
		timer -= dT;
		if (timer < 0 && UI.mouseNotIn("itemMenu") ) {
			pWin->setVisible(false);
			currentItem = 0;
		}
	}
}

/** Catch a click on drop/examine, etc. */
void CItemMenu::onRichTextClick(const std::string& msg) {
	if (msg == "drop") {
		game.player->playerC->dropItem(currentItem);
		//CItem* item = (CItem*)game.getEntity(currentItem);
		//item->drop();
	} 
	else if (msg == "examine") {
		CItem* item = (CItem*)game.getEntity(currentItem);
		item->examine();
	} else if (msg == "take") {
		CItem* item = (CItem*)game.getEntity(currentItem);
		item->take(game.player);
	}
	pWin->setVisible(false);
}

