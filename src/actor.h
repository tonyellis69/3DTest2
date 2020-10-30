#pragma once

#define _USE_MATH_DEFINES

#include "gameMsg.h"

#include "gamehextObj.h"



enum TActorBlock {notBlocked, currentBlocked, permBlocked, unknownBlocked};

/** A class encapuslating the movement code for game entities. */
class CHexActor : public CGameHexObj {
public:
	virtual bool update(float dT);
	bool isActor() { return true; }
	virtual int getMissileDamage() { return 0; }

	int getAction() {
		return action;
	}

	bool earlyExit = false;
	int earlyExitAction;
	CGameHexObj* earlyExitTarget;

protected:
	bool navigatePath(float dT);
	bool meleeAttack(float dT);
	bool shootTarget(float dT);
	bool turnTo(float dT);



	virtual void hitTarget() {};

//low level funcs

	bool isFacing(CHex& hex);
	bool moveTo(CHex& hex);


	bool lungeAt(CHex& hex);

	void trackPoint(glm::vec3& point);


	float dT;
	int action = tig::actNone;

	float moveSpeed2 = 7.0f;
	float turnSpeed = 10;
	bool destHexClaimed = false;
	float blockedFor;

	float animCycle;
	CHex targetHex; ///<Target hex, if any, for the current action.
	CGameHexObj* actionTarget; ///<Subject, if any, for the current action


	//tracking stuff
	CHexActor* trackingTarget = NULL;
	CHex lastSeen;
	bool canSeePlayer = false;

private:
	virtual void onMovedHex();

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

class CGetPlayerObj : public CMsg {
public:
	CGetPlayerObj() {}

	CHexActor* playerObj;
};

