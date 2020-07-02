#include "hexItem.h"


CHexItem::CHexItem() {
	mBlocks = blocksNone;
}

void CHexItem::leftClick() {
	CGetPlayerPos msg;
	send(msg);

	if (isNeighbour(msg.position)) {
		CTakeItem takeMsg(this);
		send(takeMsg);
	}

}

/** Respond to another item being dropped on us. */
void CHexItem::droppedOnBy(CGameHexObj& item) {
	CGetPlayerPos posMsg;
	send(posMsg);

	CCreateGroupItem msg((CHexItem*)&item, this, posMsg.position);
	send(msg);
}
