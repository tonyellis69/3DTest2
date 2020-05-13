#include "groupItem.h"

#include "utils/log.h"

CGroupItem::CGroupItem() {
	mBlocks = false;
}

/** Respond to another item being dropped on us. */
void CGroupItem::droppedOnBy(CGameHexObj& item) {
	items.push_back(&item);

}

bool CGroupItem::onLeftClick() {
	liveLog << "\n";
	for (auto item : items) {
		liveLog << item->tigMemberString(tig::name);

	}
	return true;
}

CGameHexObj* CGroupItem::removeItem(int itemNo) {
	CGameHexObj* item = items[itemNo];
	items.erase(items.begin() + itemNo);
	if (items.empty())
		deleteMe = true;
	return item;
}
