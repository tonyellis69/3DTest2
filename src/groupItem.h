#pragma once

#include <vector>

#include "gamehextObj.h"


/** A class grouping multiple items occupying the same hex. */
class CGroupItem : public CGameHexObj {
public:
	CGroupItem();
	void droppedOnBy(CGameHexObj& item);
	bool onLeftClick();
	CGameHexObj* removeItem(int itemNo);

	std::vector<CGameHexObj*> items;

};