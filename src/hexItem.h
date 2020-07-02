#pragma once

#include "gamehextObj.h"

/** A basic class for items in the game. */
class CHexItem : public CGameHexObj {
public:
	CHexItem();
	void leftClick();
	void droppedOnBy(CGameHexObj& item);
};

class CTakeItem : public CMsg {
public:
	CTakeItem(CHexItem* i) : item(i) {}

	CHexItem* item;
};

class CDropItem : public CMsg {
public:
	CDropItem(CHexItem* i, CHex& l) : item(i), location(l) {}

	CHexItem* item;
	CHex location;
};

class CCreateGroupItem : public CMsg {
public:
	CCreateGroupItem(CHexItem* i1, CHexItem* i2, CHex& l) 
		: item1(i1), item2(i2), location(l) {}

	CHexItem* item1;
	CHexItem* item2;
	CHex location;
};

