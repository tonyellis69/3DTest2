#pragma once

#define _USE_MATH_DEFINES

#include "gameMsg.h"

#include "gamehextObj.h"

enum TActorBlock {notBlocked, currentBlocked, permBlocked};

/** A class encapuslating the movement code for game entities. */
class CHexActor : public CGameHexObj {
public:
	void startAction();
	bool update2(float dT);
	bool isActor() { return true; }

protected:
	bool navigatePath(float dT);


//low level funcs
	bool isFacing(CHex& hex);
	bool moveTo(CHex& hex);
	void updateMapPos(CHex& newHex);
	TActorBlock isBlocked(CHex& hex);


	float dT;
	int action = tig::actNone;
	THexList travelPath2; ///<A sequence of hexes to travel down.
	float moveSpeed2 = 7.0f;
	float turnSpeed = 10;
};

enum TAction {actionSerial, actionSimul};
class CAddActor : public CMsg {
public:
	CAddActor(CHexActor* add, TAction listType) : actor(add),
	 addTo(listType) {};
	CHexActor* actor;
	TAction addTo;
};

class CActorBlock : public CMsg {
public:
	CActorBlock(CHex& h) : hex(h) {}
	CHex hex;
	CHexActor* blockingActor = NULL;
};

