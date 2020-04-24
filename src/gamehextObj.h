#pragma once

#define _USE_MATH_DEFINES //for cmath

#include <stack>

#include "hex/hexObject.h"
#include "../VMtest/src/ITigObj.h"

#include "tigConst.h"
#include "tigExport.h"

class CGameHexObj;
class CAction {
public:
	int actionId;
	CGameHexObj* object; ///<Target of action, if any.
};

class IHexWorld;
class IGameHexArray;
/** Basic hex object to derive game hex objects from,
	such as robots. */
class CGameHexObj : public CHexObject, public CTigObjptr{
public:
	CGameHexObj();
	void setMap(IGameHexArray* map);
	void calcTravelPath(CHex& target);
	bool beginMove();
	virtual void stackAction(CAction chosenAction) {};
	virtual void receiveDamage(CGameHexObj& attacker, int damage);
	virtual void chooseTurnAction() {};
	static void setHexWorld(IHexWorld* obj);
	virtual bool isResolvingSerialAction();
	virtual void beginTurnAction() {};
	virtual void beginLunge(CGameHexObj& target) {};
	virtual void hitTarget();
	virtual void beginTurnToTarget(CHex& target);
	bool isNeighbour(CGameHexObj& obj);
	int getChosenAction();
	std::string getName();
	virtual bool onLeftClick() { return false; };
	virtual void takeItem(CGameHexObj& item) {};
	virtual void droppedOnBy(CGameHexObj& item) {};
	virtual void onMouseOver();

	virtual void frameUpdate(float dT) {};


	bool isRobot;

	bool blocks; ///<If true, blocks travel path
	bool deleteMe;

	IGameHexArray* map; ///<The map this object exists in.


protected:
	virtual bool updateLunge(float dT);
	int tigCall(int memberId) { return 0; };

	inline static IHexWorld* hexWorld;

	CGameHexObj* attackTarget;

	int shields[6] = { 0 }; //TO DO: might not suit base class

	float lungeSpeed;

	int meleeDamage;


	std::stack<CAction> actions; ///<Actions to perform this turn, in order.
	CAction currentAction; ///<The current action being resolved, if any.

private:
	virtual void beginAttack(CHexObject& target) {};
};

using TEntities = std::vector<CGameHexObj*>;

class CGroupItem;

