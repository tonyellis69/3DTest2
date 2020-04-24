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
	virtual CGameHexObj* getBlockingEntityAt(CHex& hex) = 0;
	virtual CGameHexObj* getItemAt(CHex& position) = 0;
	virtual bool isBlockerMovingTo(CHex& hex) = 0;
	virtual CGameHexObj* getPlayerObj() = 0;
	virtual CHexObject* getCursorObj() = 0;
	virtual CHex getPlayerPosition() = 0;
	virtual CHex getPlayerDestination() = 0;
	//virtual void playerTake(CGameHexObj& item) = 0;
	virtual void dropItem(CGameHexObj* item, CHex& location) = 0;
	virtual CGroupItem* createGroupItem() = 0;
	virtual void removeEntity(CGameHexObj& entity) = 0;
	virtual void removeGridObj(CGridObj& gridObj) = 0;
	virtual void addToSerialActions(CGameHexObj* entity) = 0;
	//virtual void addSerialAction(CGameHexObj* entity, CAction action) = 0;
	virtual CGridObj* createBolt() = 0;
};
