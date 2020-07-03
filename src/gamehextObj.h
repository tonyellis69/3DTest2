#pragma once

#define _USE_MATH_DEFINES //for cmath

#include <stack>

#include "hex/hexObject.h"
#include "../VMtest/src/ITigObj.h"

#include "tigConst.h"
#include "tigExport.h"

#include "gameEvents.h"
#include "messaging/messenger.h"
#include "gameMsg.h"

const bool resolved = false;
const bool unresolved = true;


/** Basic hex object to derive game hex objects from,
	such as robots. */
class CGameHexObj : public CHexObject, public CTigObjptr,
	public CGameEventObserver, public CMessenger
{
public:
	CGameHexObj();

	virtual void receiveDamage(CGameHexObj& attacker, int damage);

	virtual void beginTurnAction() {};
	virtual void beginLunge(CGameHexObj& target) {};

	
	bool isNeighbour(CGameHexObj& obj);
	bool isNeighbour(CHex& hex);
	int getChosenAction();
	std::string getName();
	virtual void takeItem(CGameHexObj& item) {};
	virtual void droppedOnBy(CGameHexObj& item) {};

	virtual void frameUpdate(float dT) {};
	virtual unsigned int blocks() ;
	bool blocks(THexDir direction);

	virtual bool isActor() { return false;  }

	virtual void leftClick() {}

	unsigned int mBlocks; ///<If true, blocks travel path
	bool deleteMe;

	

protected:
	int tigCall(int memberId) { return 0; };

	float lungeSpeed;
	int meleeDamage;


private:
	
};

using TEntities = std::vector<CGameHexObj*>;

class CGroupItem;

class CMoveEntity : public CMsg {
public:
	CMoveEntity(CGameHexObj* ent, CHex& hex) : entity(ent), newHex(hex) {}

	CGameHexObj* entity;
	CHex& newHex;
};

class CRemoveEntity : public CMsg {
public:
	CRemoveEntity(CGameHexObj* e) : entity(e) {}

	CGameHexObj* entity;
};

class CGetPlayerObj : public CMsg {
public:
	CGetPlayerObj() {}

	CGameHexObj* playerObj;
};

class CGetObjectAt : public CMsg {
public:
	CGetObjectAt(CHex& h, CGameHexObj* n = NULL)
		: hex(h), notObj(n) {}

	CHex hex;
	CGameHexObj* obj = NULL;
	CGameHexObj* notObj;
};