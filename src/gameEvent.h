#pragma once

#include "hex/hexArray.h"

enum TGameEvent{gameLevelChange, gamePlayerDeath};
class CGameEvent {
public:
	CGameEvent() {};
	TGameEvent type;

	CHexArray* hexArray;
};
