
#include "item.h"

#include "..\3Dtest\src\hexRenderer.h"

#include "..\playerHexObj.h"

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

