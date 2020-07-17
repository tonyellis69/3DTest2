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

	bool reduceHitPoints(int damage);

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
	virtual void leftClickPowerMode() {}

	unsigned int mBlocks; ///<If true, blocks travel path
	bool deleteMe;

	

protected:
	int tigCall(int memberId) { return 0; };

	float lungeSpeed;
	int meleeDamage;

	int tmpHP; //temp hitpoints
	int tmpOrigHP; //temp starting hitpoints, for reference

private:
	virtual void deathRoutine() {}
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

class CSetPlayerAction : public CMsg {
public:
	CSetPlayerAction(int actId, CGameHexObj* obj = NULL, CHex t = CHex(-1, -1, -1))
		: action(actId), targetObj(obj), targetHex(t) {}
	int action;
	CHex targetHex;
	CGameHexObj* targetObj;
};

class CShootAt : public CMsg {
public:
	CShootAt(CHex& s, CHex& t, CGameHexObj* a, int d) : start(s), target(t),
		attacker(a), damage(d) {}
	CHex start;
	CHex target;
	CGameHexObj* attacker;
	int damage;
};


class CGetObjectAt : public CMsg {
public:
	CGetObjectAt(CHex& h, CGameHexObj* n = NULL)
		: hex(h), notObj(n) {}

	CHex hex;
	CGameHexObj* obj = NULL;
	CGameHexObj* notObj;
};

class CReserveNextPower : public CMsg {
public:
	CReserveNextPower(CGameHexObj* e) : reserver(e) {}

	CGameHexObj* reserver;
};

class CKill : public CMsg {
public:
	CKill(CGameHexObj* e) : entity(e) {}

	CGameHexObj* entity;
};

class CFindPowerUser : public CMsg {
public:
	CFindPowerUser(CGameHexObj* e, bool forceAssign = false) : user(e),
		forced(forceAssign) {}

	CGameHexObj* user;
	int power = 0;
	bool forced;
};

