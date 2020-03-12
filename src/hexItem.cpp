#include "hexItem.h"
#include "groupItem.h"

#include "IHexWorld.h"

CHexItem::CHexItem() {
	blocks = false;
}

void CHexItem::onLeftClick() {
	if (isNeighbour(*hexWorld->getPlayerObj())) {
		hexWorld->getPlayerObj()->takeItem(*this);
	}
}

/** Respond to another item being dropped on us. */
void CHexItem::droppedOnBy(CGameHexObj& item) {
	CGroupItem* groupItem = hexWorld->createGroupItem();
	groupItem->items.push_back(&item);
	groupItem->items.push_back(this);
	hexWorld->removeEntity(*this);
	groupItem->setPosition(hexWorld->getPlayerPosition());
}
