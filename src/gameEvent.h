#pragma once

#include "hex/hexArray.h"


enum TGameEvent{gameLevelChange, gamePlayerSpawn, gamePlayerDeath};

class CEntity;
class CGameEvent {
public:
	CGameEvent(TGameEvent _type) : type(_type) {};
	TGameEvent type;

	CHexArray* hexArray;
	CEntity* entity;
};
