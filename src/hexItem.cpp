#include "hexItem.h"

CHexItem::CHexItem() {
	blocks = false;
}

void CHexItem::onLeftClick() {
	if (isNeighbour(*hexWorld->getPlayerObj())) {
		hexWorld->getPlayerObj()->takeItem(*this);

	}
}
