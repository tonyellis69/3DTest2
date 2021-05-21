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

#include "physics/phys.h"

const bool resolved = false;
const bool unresolved = true;

enum TMoveDir {
	moveNone, moveEast, moveWest, moveNE, moveSE,
	moveSW, moveNW, moveNorth, moveSouth, moveNS2, moveNS2blocked
};

enum TEntityType { entNone, entRobot};

/** Basic hex object to derive game hex objects from,
	such as robots. */
class CGameHexObj : public CHexObject, public CTigObjptr,
	public CGameEventObserver, public CMessenger
{
public:
	CGameHexObj();

	void setLineModel(const std::string& name);

	void draw();

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

	virtual void playerSight(bool inView);

	virtual void update(float dT) {}

	virtual void onMovedHex() {};

	virtual std::tuple<bool, glm::vec3> collisionCheck(glm::vec3& segA, glm::vec3& segB) {
		return std::tuple<bool, glm::vec3>();
	}

	float orientationTo(glm::vec3& targetPos);
	void updatePos(glm::vec3& dPos);


	unsigned int mBlocks; ///<If true, blocks travel path
	bool deleteMe;

	bool visibleToPlayer = false; ///<True if object in player line of sight

	glm::vec4 normalColour;

	CHex moveDest; ///<Adjacent hex we're moving to.
	CHex moveDest2; ///<Second hex if travelling north/south
	//TO DO: consider moving to CHexActor
	
	TMoveDir travelDir = moveNone;

	TEntityType entityType = entNone;

	int tmpId = 0;

	CPhys physics;

protected:
	int tigCall(int memberId) { return 0; };

	float lungeSpeed;
	int meleeDamage;

	int tmpHP; //temp hitpoints
	int tmpOrigHP; //temp starting hitpoints, for reference

private:
	virtual void deathRoutine() {}
};

//using TEntities = std::vector<CGameHexObj*>;
using TEntity = std::shared_ptr<CGameHexObj>;
using TEntities = std::vector<std::shared_ptr<CGameHexObj>>;

class CGroupItem;



class CRemoveEntity : public CMsg {
public:
	CRemoveEntity(CGameHexObj* e) : entity(e) {}

	CGameHexObj* entity;
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



class CReserveNextPower : public CMsg {
public:
	CReserveNextPower(CGameHexObj* e) : reserver(e) {}

	CGameHexObj* reserver;
};



class CFindPowerUser : public CMsg {
public:
	CFindPowerUser(CGameHexObj* e, bool forceAssign = false) : user(e),
		forced(forceAssign) {}

	CGameHexObj* user;
	int power = 0;
	bool forced;
};

