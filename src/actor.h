#pragma once

#define _USE_MATH_DEFINES

#include "gameMsg.h"

#include "gamehextObj.h"



enum TActorBlock {notBlocked, currentBlocked, permBlocked, unknownBlocked};

/** A class encapuslating the movement code for game entities. */
class CHexActor : public CGameHexObj {
public:
	virtual void chooseTurnAction() {}
	void setAction(int actId, CGameHexObj* target = NULL);
	void setAction(int actId, CHex& targetHex);
	virtual void initAction();
	virtual bool update(float dT);
	bool isActor() { return true; }
	virtual int getMissileDamage() { return 0; }

protected:
	bool navigatePath(float dT);
	bool meleeAttack(float dT);
	bool shootTarget(float dT);

	bool checkForBlock(CHex& destHex);

	virtual void hitTarget() {};

//low level funcs

	bool isFacing(CHex& hex);
	bool moveTo(CHex& hex);
	void claimMapPos(CHex& newHex);

	bool lungeAt(CHex& hex);



	float dT;
	int action = tig::actNone;

	float moveSpeed2 = 7.0f;
	float turnSpeed = 10;
	int movePoints2; ///<Number of hexes we can travel in one move.
	bool destHexClaimed = false;
	float blockedFor;

	float animCycle;
	CHex targetHex; ///<Target hex, if any, for the current action.
	CGameHexObj* actionTarget; ///<Subject, if any, for the current action


};

enum TAction {actionSerial, actionSimul};
class CAddActor : public CMsg {
public:
	CAddActor(CHexActor* add, TAction listType) : actor(add),
	 addTo(listType) {};

	CHexActor* actor;
	TAction addTo;
};

class CFindActorBlock : public CMsg {
public:
	CFindActorBlock(CHex& h) : hex(h) {}
	CHex hex;
	CHexActor* blockingActor = NULL;
};

class CGetActorAt : public CMsg {
public:
	CGetActorAt(CHex& h, CHexActor* n = NULL)
		: hex(h), notActor(n) {}

	CHex hex;
	CHexActor* actor = NULL;
	CHexActor* notActor;
};

//enum TAttack {attackMelee, attackBolt};
//class CReceiveDamage : public CMsg {
//public:
//	CReceiveDamage(CHexActor* actor) : attacker(actor) {}
//
//	CHexActor* attacker;
//	int damage;
//	TAttack attack;
//};