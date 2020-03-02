#pragma once

#include "gamehextObj.h"

/** A basic class for items in the game. */
class CHexItem : public CGameHexObj {
public:
	CHexItem();
private:
	void onLeftClick();



};
