#include "itemMenu.h"

#include "../items/item.h"
#include "../gameState.h"
#include "../gameTextWin.h"
#include "../gameGui.h"

#include "UI/gui2.h"

#include "utils/log.h"

void CItemMenu::onEvent(CEvent& e) {
	if (e.type == eHotTextHover && gui.mouseIn("inv") ) {
		if (e.hotTxt->empty()) 
			startTimeout();
		else
			showItemChoices(*e.hotTxt);
	}
	else if (e.type == eMouseOff  && gui.mouseNotIn("itemMenu")
		 && gui.mouseNotIn("itemMenu") ) {
		startTimeout();
		liveLog << " start timeout ";
	}
	//else if (e.type == eMouseMove && gui.mouseIn("itemMenu")) {
	//	timer = 0;
	//	liveLog << " stop timeout ";
	//}
}

void CItemMenu::startTimeout() {
	if (timer <= 0 && pWin->visible)
		timer = 0.5f;
}

void CItemMenu::showItemChoices(const std::string& hotTxt) {
	int entityNo = std::stoi(hotTxt.substr(hotTxt.find_first_of("0123456789")).c_str());

	CItem* item = (CItem*)game.map->getEntity(entityNo);

	//gWin::putLeftOf("itemMenu", "inv");
	positionLeftOf(gui.findControl("inv"));

	//get this item's menu text
	std::string menuTxt;
	menuTxt = item->getMenuTextInv();

	//display it
	pWin->clearText();
	pWin->addText(menuTxt);

	gWin::alignWithMouse("itemMenu");
	pWin->setVisible(true);
	timer = 0;
}


void CItemMenu::positionLeftOf(CguiBase* spawner) {
	//find left edge of parent
	glm::i32vec2 pos = spawner->getLocalPos();
	glm::i32vec2 size = pWin->getSize();

	//position back from that
	pWin->anchorLeft = NONE;
	pWin->setPosX(pos.x - size.x - 10);
}

void CItemMenu::update(float dT) {
	if (pWin->visible == false )
		return;

	//if (pWin->isMouseOver() && timer > 0) {
	//	timer = 0;
	//	liveLog << " stop timeout ";
	//	return;
	//}

	if (timer > 0 ) { 
		timer -= dT;
		if (timer < 0 && gui.mouseNotIn("itemMenu") ) {
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

