#pragma once

#include "hex/hexObject.h"
#include "../VMtest/src/ITigObj.h"

#include "tigConst.h"
#include "tigExport.h"

class IhexObjectCallback;
/** Basic hex object to derive game hex objects from,
	such as robots. */
class CGameHexObj : public CHexObject, public CTigObjptr{
public:
	CGameHexObj();
	void calcTravelPath(CHex& target);
	bool beginMove();
	virtual void receiveDamage(CGameHexObj& attacker, int damage);
	virtual void chooseTurnAction() {};
	static void setHexWorld(IhexObjectCallback* obj);
	virtual bool isResolvingSerialAction();
	virtual void beginTurnAction() {};
	virtual void hitTarget();
	bool isNeighbour(CGameHexObj& obj);
	int getCurrentAction();
	std::string getName();
	virtual void onLeftClick() {};
	virtual void takeItem(CGameHexObj& item) {};
	bool isRobot;

	bool blocks; ///<If true, blocks travel path

protected:
	virtual bool updateLunge(float dT);
	int tigCall(int memberId) { return 0; };

	inline static IhexObjectCallback* hexWorld;

	CGameHexObj* attackTarget;

	int shields[6] = { 0 }; //TO DO: might not suit base class

	float lungeSpeed;

	int meleeDamage;


private:
	virtual void beginAttack(CHexObject& target) {};
};

using TEntities = std::vector<CGameHexObj*>;

class IhexObjectCallback {
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
};
