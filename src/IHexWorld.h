#pragma once

#include "hex/hex.h"
#include "hex/hexObject.h"

//#include "gamehextObj.h"
//#include "gridObj.h"

class CGameHexObj;
class CGridObj;
class IHexWorld {
public:
	virtual THexList calcPath(CHex& start, CHex& end) = 0;
	virtual CGameHexObj* getEntityAt(CHex& hex) = 0;
	virtual bool isBlockerMovingTo(CHex& hex) = 0;
	virtual void onPlayerTurnDoneCB() = 0;
	virtual CGameHexObj* getPlayerObj() = 0;
	virtual CHex getPlayerPosition() = 0;
	virtual CHex getPlayerDestinationCB() = 0;
	virtual bool isEntityDestinationCB(CHex& hex) = 0;
	virtual int diceRoll(int dice) = 0;
	virtual void playerTake(CGameHexObj& item) = 0;
	virtual void beginPlayerLunge(CGameHexObj& target) = 0;
	virtual void playerDrop(CGameHexObj* item) = 0;
	virtual CGroupItem* createGroupItem() = 0;
	virtual void removeEntity(CGameHexObj& entity) = 0;
	virtual void removeGridObj(CGridObj& gridObj) = 0;
};
