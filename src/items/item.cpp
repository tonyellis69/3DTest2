
#include "item.h"

//#include "..\3Dtest\src\hexRenderer.h"

#include "..\playerHexObj.h"

#include "..\gameGui.h"

#include "..\hexRender\itemDraw.h"

void CItem::setParent(CEntity* parent) {
	this->parent = parent;
}


void CItem::drop() {
	//drawFn = std::make_shared<CItemDraw>(this);
	//TO DO: rewrite to work with component?
}

void CItem::take(CEntity* taker) {
	//TO DO: assumption! Taker may not always be the player. Amend if necessary.
	CPlayerObject* takerEnt = (CPlayerObject*) taker;
	takerEnt->addToInventory(this);
	gWin::pNear->removeItem(id);
	//drawFn = std::make_shared<CDrawFunc>(nullptr);
	//TO DO: needs component-handling
}

void CItem::examine() {
	std::string desc = name + ":\nDescription text";
	gWin::pExam->clearText();
	gWin::pExam->addText( desc);
	gWin::pExam->showWin();
}



std::string CItem::getMenuTextInv() {
	return name + ":\n\\h{drop}Drop\\h\n\\h{examine}Examine\\h";
}

std::string CItem::getMenuTextNear() {
	return name + ":\n\\h{take}Take\\h\n\\h{examine}Examine\\h";
}

