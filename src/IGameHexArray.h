#pragma once 

#include "hex/hex.h"

class CGameHexObj;

class IGameHexArray {
public:
	virtual CHex findLineEnd(CHex& start, CHex& target) = 0;
	virtual void moveEntity(CGameHexObj* entity, CHex& hex) = 0;
	virtual CGameHexObj* getEntityAt(CHex& hex) = 0;
	virtual CGameHexObj* getBlockingEntityAt(CHex& hex) = 0;
	virtual CGameHexObj* getEntityNotSelf(CGameHexObj* self) = 0;

};