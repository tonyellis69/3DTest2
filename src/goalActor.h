#pragma once

#include "actor.h"

/** Encapsulates the functionality of multi-turn goals, such as 
	stalking the player or operating machinery.*/
class CGoalActor : public CHexActor {
public:
	CGoalActor() {}
	void setGoalWander();
	void setGoalAttack(CGameHexObj* target);
	void setGoalGotoLastSeen(CHex& dest, CGameHexObj* target);

	int getGoal();







	virtual bool canSee(CGameHexObj* target) = 0;
	virtual CHex getLastSeen() = 0;

	int goal;
	CHex goalDestinationHex;
	CGameHexObj* goalTarget;
};