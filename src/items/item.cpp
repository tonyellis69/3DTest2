
#include "item.h"

#include "..\3Dtest\src\hexRenderer.h"

#include "..\playerHexObj.h"

#include "..\gameGui.h"

void CItem::setParent(CEntity* parent) {
	this->parent = parent;
}

void CItem::setModel(CModel& model) {
	this->model = model;
	setBoundingRadius();


}




void CItem::draw() {
	if (parent == nullptr) {
		hexRendr2.drawSolidModel(model.meshes[0]);
		hexRendr2.drawSolidModel(model.meshes[1]);
		hexRendr2.drawLineModel(model.meshes[2]);
	}
}

void CItem::take(CEntity* taker) {
	//TO DO: assumption! Taker may not always be the player. Amend if necessary.
	CPlayerObject* takerEnt = (CPlayerObject*) taker;
	takerEnt->addToInventory(this);

}


void CItem::onMouseover(const std::string& parentWin) {
	//open item menu window
	if (parentWin == "inv" && !itemMenuOpen) {
		gWin::putLeftOf("itemMenu", "inv");
		gWin::alignWithMouse("itemMenu");
		itemMenuOpen = true;
	}
		
	gWin::clearText("itemMenu");
	std::string txt =  name + ":\n\\h{drop}Drop\\h\n\\h{examine}Examine\\h";
	gWin::addText("itemMenu", txt);
	
	gWin::showWin("itemMenu");
	

	//write options

}

void CItem::loseItemMenu() {
	itemMenuOpen = false;
	gWin::hideWin("itemMenu");
}

void CItem::timeOutItemMenu() {
	itemMenuOpen = false;
	gWin::timeOut("itemMenu",0.5f);
}

