#pragma once

#include "hex/hexObject.h"
#include "../VMtest/src/ITigObj.h"


#include "tigConst.h"
#include "tigExport.h"

class IHexWorld;
/** Basic hex object to derive game hex objects from,
	such as robots. */
class CGameHexObj : public CHexObject, public CTigObjptr{
public:
	CGameHexObj();
	void calcTravelPath(CHex& target);
	bool beginMove();
	virtual void receiveDamage(CGameHexObj& attacker, int damage);
	virtual void chooseTurnAction() {};
	static void setHexWorld(IHexWorld* obj);
	virtual bool isResolvingSerialAction();
	virtual void beginTurnAction() {};
	virtual void hitTarget();
	bool isNeighbour(CGameHexObj& obj);
	int getCurrentAction();
	std::string getName();
	virtual void onLeftClick() {};
	virtual void takeItem(CGameHexObj& item) {};
	virtual void droppedOnBy(CGameHexObj& item) {};


	bool isRobot;

	bool blocks; ///<If true, blocks travel path
	bool deleteMe;

protected:
	virtual bool updateLunge(float dT);
	int tigCall(int memberId) { return 0; };

	inline static IHexWorld* hexWorld;

	CGameHexObj* attackTarget;

	int shields[6] = { 0 }; //TO DO: might not suit base class

	float lungeSpeed;

	int meleeDamage;


private:
	virtual void beginAttack(CHexObject& target) {};
};

using TEntities = std::vector<CGameHexObj*>;

class CGroupItem;

